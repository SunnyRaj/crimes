#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define P2M_SIZE_1GB 262144
#define AVG_DIRTY_PAGES 600

#define BITMAP_ENTRY(_nr,_bmap) ((_bmap))[(_nr) / 8]
#define BITMAP_SHIFT(_nr) ((_nr) % 8)

/* Timer related variables */
typedef long long NANOSECONDS;
typedef struct timespec TIMESPEC;

static inline NANOSECONDS ns_timer(void)
{
    TIMESPEC curr_time;
    clock_gettime(CLOCK_MONOTONIC, &curr_time);
    return (NANOSECONDS) (curr_time.tv_sec * 1000000000LL) +
        (NANOSECONDS) (curr_time.tv_nsec);
}

static inline int test_bit(int nr, const void *_addr)
{
    const char *addr = _addr;
    return (BITMAP_ENTRY(nr, addr) >> BITMAP_SHIFT(nr)) & 1;
}

static inline void set_bit(int nr, void *_addr)
{
    char *addr = _addr;
    BITMAP_ENTRY(nr, addr) |= (1UL << BITMAP_SHIFT(nr));
}

static inline int bitmap_size(int nr_bits)
{
    return (nr_bits + 7) / 8;
}

static inline void bitmap_clear(void *addr, int nr_bits)
{
    memset(addr, 0, bitmap_size(nr_bits));
}

void set_random_bits ( void *bmap, size_t P2M_SIZE)
{
    unsigned i;
    srand(time(NULL));
    for ( i = 0; i < AVG_DIRTY_PAGES; i++ )
        set_bit(rand() % (P2M_SIZE), bmap);
}

int main (void)
{
    size_t iter;
    for ( iter = 1; iter <= 16; iter++)
    {
        size_t P2M_SIZE = P2M_SIZE_1GB * iter;
        char bitmap[P2M_SIZE/8];
        unsigned long i, j, start, end, dirty_bits = 0;

        bitmap_clear ( bitmap, P2M_SIZE );

        set_random_bits( bitmap, P2M_SIZE );

        start = ns_timer();
        dirty_bits = 0;
        for ( i = 0; i < P2M_SIZE; i++ )
            if ( test_bit(i, bitmap) )
                dirty_bits++;
        end = ns_timer();

        /* printf("Bit: Actual # dirty pages: %ld pages\n", dirty_bits); */
        printf("Bit: Time taken for %zu: %f ms\n", iter, (end - start)/(float)1000000);

        size_t sz_c = sizeof(char) * 8;

        start = ns_timer();
        dirty_bits = 0;
        for ( i = 0; i < P2M_SIZE/sz_c; i++ )
        {
            if (bitmap[i] == 0)
                continue;

            for ( j = 0; j < sz_c; j++)
            {
                if ( !test_bit((i*sz_c)+j, bitmap) )
                    continue;
                dirty_bits++;
            }
        }
        end = ns_timer();

        /* printf("Byte: # dirty pages: %ld pages\n", dirty_bits); */
        printf("Byte: Time taken for %zu: %f ms\n", iter, (end - start)/(float)1000000);

    }

    return 0;
}
