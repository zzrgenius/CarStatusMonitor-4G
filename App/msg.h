#ifndef MSG_H
#define MSG_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t type;              /**< Type field. */
	uint16_t length;				// rec data length
    void *ptr;              /**< Pointer content field. */
} msg_t;


#endif
