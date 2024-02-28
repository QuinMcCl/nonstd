#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SWAPCHAIN_READ_CLOSE(CHAIN, LINK) CHAIN.read_close(&(CHAIN), LINK)
#define SWAPCHAIN_READ_FRONT(CHAIN, LINK, DATA) CHAIN.read_front(&(CHAIN), &LINK, (const void **)&DATA)
#define SWAPCHAIN_READ_NEXT(CHAIN, LINK, DATA) CHAIN.read_next(&(CHAIN), &LINK, (const void **)&DATA)
#define SWAPCHAIN_WRITE_CLOSE(CHAIN, LINK) CHAIN.write_close(&(CHAIN), LINK)
#define SWAPCHAIN_WRITE_BACK(CHAIN, LINK, DATA) CHAIN.write_back(&(CHAIN), &LINK, (const void **)&DATA)

    typedef struct swapchain_s swapchain_t;
    typedef struct swaplink_s swaplink_t;

    typedef int (*swapchain_read_close_func_t)(swapchain_t *chain, swaplink_t *link_ptr);
    typedef int (*swapchain_read_front_func_t)(swapchain_t *chain, swaplink_t **link_ptr, const void **data);
    typedef int (*swapchain_read_next_func_t)(swapchain_t *chain, swaplink_t **link_ptr, const void **data);
    typedef int (*swapchain_write_close_func_t)(swapchain_t *chain, swaplink_t *link_ptr);
    typedef int (*swapchain_write_back_func_t)(swapchain_t *chain, swaplink_t **link_ptr, const void **data);

    struct swaplink_s
    {
        pthread_mutex_t newer_rwlock;
        pthread_rwlock_t data_rwlock;
        swaplink_t *newer;
        unsigned char data[];
    };

    struct swapchain_s
    {
        pthread_rwlock_t newest_rwlock;
        pthread_mutex_t oldest_rwlock;
        pthread_cond_t done_read_cond;
        swaplink_t *newest;
        swaplink_t *oldest;
        swaplink_t *swaplink_array;
        swapchain_read_close_func_t read_close;
        swapchain_read_front_func_t read_front;
        swapchain_read_next_func_t read_next;
        swapchain_write_close_func_t write_close;
        swapchain_write_back_func_t write_back;
    };

    int swapchain_init(
        swapchain_t *chain,
        size_t backing_buffer_length,
        void *backing_buffer,
        size_t item_size,
        size_t chunk_alignment,
        swapchain_read_close_func_t read_close_fun,
        swapchain_read_front_func_t read_front_fun,
        swapchain_read_next_func_t read_next_fun,
        swapchain_write_close_func_t write_close_fun,
        swapchain_write_back_func_t write_back_fun);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif