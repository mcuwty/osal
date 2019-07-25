#include "osal_memory.h"
#include "type.h"

#if ( MAXMEMHEAP >= 32768 )             //内存管理默认使用15位数据标识，最大能管理32768字节
#error MAXMEMHEAP is too big to manage!
#endif

//查找到合适的内存块之后，就要决定是否对此内存块进行分割。
//如果内存块过大的话必然会造成内存的浪费。
//如果内存块的大小减去要申请的内存块的值大于OSALMEM_MIN_BLKSZ(4byte)，
//则分割此内存块。并初使化分割出来的内存分配控制块头。
#if !defined ( OSALMEM_MIN_BLKSZ )
#define OSALMEM_MIN_BLKSZ    4
#endif

#if !defined ( OSALMEM_SMALL_BLKSZ )
#define OSALMEM_SMALL_BLKSZ  16     //固定内存分配区域的固定长度，16字节
#endif

#if !defined ( OSALMEM_GUARD )
#define OSALMEM_GUARD  TRUE         // TBD - Hacky workaround til Bugzilla 1252 is fixed!
#define OSALMEM_READY  0xE2
#endif

//内存分配返回的是一个指向分配区域的指针，指针的长度是内存控制头和内存对齐方式中较大的一个。
//并且这个长度也是最小分配单元的长度。在OSAL中它的长度是16bit。
//此处需要根据实际编译环境修改，确保osalMemHdr_t长度为16bit或以上
typedef halDataAlign_t  osalMemHdr_t;

/*********************************************************************
 * CONSTANTS
 */

#define OSALMEM_IN_USE  0x8000        //内存控制头（16位）最高位标识该内存块是否被使用

/* This number sets the size of the small-block bucket. Although profiling
 * shows max simultaneous alloc of 16x18, timing without profiling overhead
 * shows that the best worst case is achieved with the following.
   根据程序的概要分析，最大的同时分配的大小是16x18, 如果不进行程序概要分析，
   在定时分析系统的开销时，最坏情况里面的最好情况的内存分配大小是232(byte)的长度。
   所以把固定分配区域的长度定义为232（byte）。
 */
#define SMALLBLKHEAP    232

//内存分配返回的是一个指向分配区域的指针，指针的长度是内存控制头和内存对齐方式中较大的一个。
//并且这个长度也是最小分配单元的长度。在些OSAL中它的长度是16bit。
//此处需要根据实际编译环境修改，确保osalMemHdr_t长度为16bit或以上
// To maintainalignment of the pointer returned, reserve the greater
// space for the memory block header.
#define HDRSZ  ( (sizeof ( halDataAlign_t ) > sizeof( osalMemHdr_t )) ? \
                  sizeof ( halDataAlign_t ) : sizeof( osalMemHdr_t ) )

#if ( OSALMEM_GUARD )
static byte ready = 0;
#endif

static osalMemHdr_t *ff1;  // First free block in the small-block bucket.
static osalMemHdr_t *ff2;  // First free block after the small-block bucket.

#if defined( EXTERNAL_RAM )
static byte  *theHeap = (byte *)EXT_RAM_BEG;
#else
static halDataAlign_t _theHeap[ MAXMEMHEAP / sizeof(halDataAlign_t) ];
// static __align(32) halDataAlign_t _theHeap[ MAXMEMHEAP / sizeof( halDataAlign_t ) ];
//根据实际使用的芯片设定内存对齐，__align(32)，非常重要！！！
static byte  *theHeap = (byte *)_theHeap;
#endif

#if OSALMEM_METRICS
static uint16 blkMax;  // Max cnt of all blocks ever seen at once.
static uint16 blkCnt;  // Current cnt of all blocks.
static uint16 blkFree; // Current cnt of free blocks.
static uint16 memAlo;  // Current total memory allocated.
static uint16 memMax;  // Max total memory ever allocated at once.
#endif
/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn osal_mem_init
 *
 * @brief   Initialize the heap memory management system.
 *
 * @param   void
 *
 * @return  void
 */
void osal_mem_init(void)
{
    osalMemHdr_t *tmp;
    // Setup a NULL block at the end of the heap for fast comparisons with zero.
    //整个内存池最后的两个字节清零，避免分配内存产生溢出
    tmp = (osalMemHdr_t *)theHeap + (MAXMEMHEAP / HDRSZ) - 1;
    *tmp = 0;

    // Setup a small-block bucket.
    //设置固定长度内存区长度，232字节，内存区第一个字保存该区长度
    tmp = (osalMemHdr_t *)theHeap;
    *tmp = SMALLBLKHEAP;

    // Setup the wilderness.
    //设置可变产地内存区长度，内存区第一个字保存该区长度
    tmp = (osalMemHdr_t *)theHeap + (SMALLBLKHEAP / HDRSZ);
    *tmp = ((MAXMEMHEAP / HDRSZ) * HDRSZ) - SMALLBLKHEAP - HDRSZ;

#if ( OSALMEM_GUARD )
    ready = OSALMEM_READY;
#endif

    // Setup a NULL block that is never freed so that the small-block bucket
    // is never coalesced with the wilderness.
    ff1 = tmp;
    ff2 = osal_mem_alloc(0);
    ff1 = (osalMemHdr_t *)theHeap;
    /*
    上述语句将在固定长度分配区域和可变分配区域之间申请一个0大小的内存块，
    相当于在固定分配区域和可变分配区域之间保留了一个一直处于使用状态，
    但是指向长度为0的一个内存分配控制块。
    这个内存块的是将两个分配区域隔离开，以免和可变长度分配区域合并。
    */

#if ( OSALMEM_METRICS )
    /* Start with the small-block bucket and the wilderness - don't count the
     * end-of-heap NULL block nor the end-of-small-block NULL block.
     */
    blkCnt = blkFree = 2;
    memAlo = 0;
#endif
}

/*********************************************************************
 * @fn osal_mem_kick
 *
 * @brief   Kick the ff1 pointer out past the long-lived OSAL Task blocks.
 *          Invoke this once after all long-lived blocks have been allocated -
 *          presently at the end of osal_init_system().
 *          使FF1跳过固定长度区域，指向FF2(可变长度区域)，也就是这个意思：
 *          如果在固定分配区域中没有申请内存成功，
 *          调用此函数后它将修改指向固定分配区域的指针使指针指向可变分配区域，
 *          然后再调用osal_mem_alloc 它将在可变长度分配区域中进行内存分配。
 *
 * @param   void
 *
 * @return  void
 */
void osal_mem_kick(void)
{
    //halIntState_t  intState;
    HAL_ENTER_CRITICAL_SECTION();  // Hold off interrupts.

    /* Logic in osal_mem_free() will ratchet ff1 back down to the first free
     * block in the small-block bucket.
     */
    ff1 = ff2;

    HAL_EXIT_CRITICAL_SECTION();  // Re-enable interrupts.
}

/*********************************************************************
 * @fn osal_mem_alloc
 *
 * @brief   Implementation of the allocator functionality.
 *
 * @param   size - number of bytes to allocate from the heap.
 *
 * @return  void * - pointer to the heap allocation; NULL if error or failure.
 */
void *osal_mem_alloc(uint16 size)
{
    osalMemHdr_t  *prev;
    osalMemHdr_t  *hdr;
    uint16  tmp;
    byte coal = 0;

#if ( OSALMEM_GUARD )
    // Try to protect against premature use by HAL / OSAL.
    if(ready != OSALMEM_READY)
    {
        osal_mem_init();
    }
#endif

    size += HDRSZ;

    // Calculate required bytes to add to 'size' to align to halDataAlign_t.
    //根据实际的芯片的字长halDataAlign_t进行字节对齐
    if(sizeof(halDataAlign_t) == 2)
    {
        size += (size & 0x01);
    }
    else if(sizeof(halDataAlign_t) != 1)
    {
        const byte mod = size % sizeof(halDataAlign_t);

        if(mod != 0)
        {
            size += (sizeof(halDataAlign_t) - mod);
        }
    }

    HAL_ENTER_CRITICAL_SECTION();       // Hold off interrupts.

    // Smaller allocations are first attempted in the small-block bucket.
    if(size <= OSALMEM_SMALL_BLKSZ)
    {
        hdr = ff1;
    }
    else
    {
        hdr = ff2;
    }
    tmp = *hdr;

    do
    {
        if(tmp & OSALMEM_IN_USE)
        {
            tmp ^= OSALMEM_IN_USE;          //该片已被使用，得出长度
            coal = 0;                       //找到空内存标志清零，因为遇到了一片已使用的内存
        }
        else                                //该片未被使用
        {
            if(coal != 0)                   //上轮查找有找到空内存，但空间不够
            {
#if ( OSALMEM_METRICS )
                blkCnt--;                   //内存合并，总内存块计数减一
                blkFree--;                  //内存合并，空闲内存块计数减一
#endif
                *prev += *hdr;              //加上本次找到的空内存大小

                if(*prev >= size)           //加上后内存大小符合申请需要
                {
                    hdr = prev;             //返回该内存块
                    tmp = *hdr;             //得出长度
                    break;
                }
            }
            else                            //上轮查找未找到空内存
            {
                if(tmp >= size)             //该片内存大小符合需求，跳出查找循环
                {
                    break;
                }

                coal = 1;                   //该片内存大小不符合需求，标记找到一块空内存
                prev = hdr;                 //记录该内存
            }
        }

        hdr = (osalMemHdr_t *)((byte *)hdr + tmp);  //偏移至下一片内存区域

        tmp = *hdr;                         //读取该区域长度
        if(tmp == 0)
        {
            hdr = ((void *)NULL);
            break;
        }
    } while(1);

    if(hdr != ((void *)NULL))
    {
        tmp -= size;                        //本次申请后剩余长度
        // Determine whether the threshold for splitting is met.
        if(tmp >= OSALMEM_MIN_BLKSZ)        //剩余空间大于最小需求空间，分割内存供下次申请
        {
            // Split the block before allocating it.
            osalMemHdr_t *next = (osalMemHdr_t *)((byte *)hdr + size);  //偏移
            *next = tmp;                    //记录未使用区域剩余长度
            *hdr = (size | OSALMEM_IN_USE); //标志本次申请区域已被使用，并记录本次使用长度

#if ( OSALMEM_METRICS )
            blkCnt++;                       //内存分割，总内存块计数加一
            if(blkMax < blkCnt)
            {
                blkMax = blkCnt;            //调整内存块数量最大值
            }
            memAlo += size;                 //调整已用内存大小
#endif
        }
        else
        {
#if ( OSALMEM_METRICS )
            memAlo += *hdr;
            blkFree--;                      //内存不分割，空闲内存块计数减一
#endif

            *hdr |= OSALMEM_IN_USE;
        }

#if ( OSALMEM_METRICS )
        if(memMax < memAlo)
        {
            memMax = memAlo;
        }
#endif

        hdr++;                              //偏移，返回实际申请的内存地址
    }

    HAL_EXIT_CRITICAL_SECTION();            // Re-enable interrupts.

    return (void *)hdr;
}

/*********************************************************************
 * @fn osal_mem_free
 *
 * @brief   Implementation of the de-allocator functionality.
 *
 * @param   ptr - pointer to the memory to free.
 *
 * @return  void
 */
void osal_mem_free(void *ptr)
{
    osalMemHdr_t  *currHdr;
    //halIntState_t   intState;

#if ( OSALMEM_GUARD )
    // Try to protect against premature use by HAL / OSAL.
    if(ready != OSALMEM_READY)
    {
        osal_mem_init();
    }
#endif

    HAL_ENTER_CRITICAL_SECTION();  // Hold off interrupts.

    currHdr = (osalMemHdr_t *)ptr - 1;

    *currHdr &= ~OSALMEM_IN_USE;

    if(ff1 > currHdr)
    {
        ff1 = currHdr;
    }

#if OSALMEM_METRICS
    memAlo -= *currHdr;
    blkFree++;
#endif

    HAL_EXIT_CRITICAL_SECTION();  // Re-enable interrupts.
}

#if OSALMEM_METRICS
/*********************************************************************
 * @fn osal_heap_block_max
 *
 * @brief   Return the maximum number of blocks ever allocated at once.
 *
 * @param   none
 *
 * @return  Maximum number of blocks ever allocated at once.
 */
uint16 osal_heap_block_max(void)
{
    return blkMax;
}

/*********************************************************************
 * @fn osal_heap_block_cnt
 *
 * @brief   Return the current number of blocks now allocated.
 *
 * @param   none
 *
 * @return  Current number of blocks now allocated.
 */
uint16 osal_heap_block_cnt(void)
{
    return blkCnt;
}

/*********************************************************************
 * @fn osal_heap_block_free
 *
 * @brief   Return the current number of free blocks.
 *
 * @param   none
 *
 * @return  Current number of free blocks.
 */
uint16 osal_heap_block_free(void)
{
    return blkFree;
}

/*********************************************************************
 * @fn osal_heap_mem_used
 *
 * @brief   Return the current number of bytes allocated.
 *
 * @param   none
 *
 * @return  Current number of bytes allocated.
 */
uint16 osal_heap_mem_used(void)
{
    return memAlo;
}

/*********************************************************************
 * @fn osal_heap_high_water
 *
 * @brief   Return the highest byte ever allocated in the heap.
 *
 * @param   none
 *
 * @return  Highest number of bytes ever used by the stack.
 */
uint16 osal_heap_high_water(void)
{
#if ( OSALMEM_METRICS )
    return memMax;
#else
    return MAXMEMHEAP;
#endif
}

//返回内存使用率
uint16 osal_heap_mem_usage_rate(void)
{
    return (uint16)(memAlo / (MAXMEMHEAP / 100));
}

#endif
