#ifndef MSG_H
#define MSG_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t sender_pid;    /**< PID of sending thread. Will be filled in by msg_send. */
    uint16_t type;              /**< Type field. */
        void *ptr;              /**< Pointer content field. */
} msg_t;


#endif
