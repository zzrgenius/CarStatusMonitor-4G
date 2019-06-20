#ifndef KSERVICE_H
#define KSERVICE_H


/* RT-Thread error code definitions */
#define RT_EOK                          0               /**< There is no error */
#define RT_ERROR                        1               /**< A generic error happens */
#define RT_ETIMEOUT                     2               /**< Timed out */
#define RT_EFULL                        3               /**< The resource is full */
#define RT_EEMPTY                       4               /**< The resource is empty */
#define RT_ENOMEM                       5               /**< No memory */
#define RT_ENOSYS                       6               /**< No system */
#define RT_EBUSY                        7               /**< Busy */
#define RT_EIO                          8               /**< IO error */
#define RT_EINTR                        9               /**< Interrupted system call */
#define RT_EINVAL                       10              /**< Invalid argument */


/* boolean type definitions */
#ifndef FALSE     /* in case these macros already exist */
#define FALSE 0   /* values of boolean */
#endif
#ifndef TRUE
#define TRUE  1
#endif
#define RT_NAME_MAX 8

void rt_kprintf(const char *fmt, ...);


#endif
