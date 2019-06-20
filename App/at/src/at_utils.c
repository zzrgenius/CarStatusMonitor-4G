/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-14     chenyong     first version
 */

#include <at.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
static char send_buf[AT_CMD_MAX_LEN];
static size_t last_cmd_len = 0;


/**
 * This function will calculate the tick from millisecond.
 *
 * @param ms the specified millisecond
 *           - Negative Number wait forever
 *           - Zero not wait
 *           - Max 0x7fffffff
 *
 * @return the calculated tick
 */
uint32_t rt_tick_from_millisecond(int32_t ms)
{
    uint32_t tick;

    if (ms < 0)
    {
        tick = (uint32_t)RT_WAITING_FOREVER;
    }
    else
    {
        tick = RT_TICK_PER_SECOND * (ms / 1000);
        tick += (RT_TICK_PER_SECOND * (ms % 1000) + 999) / 1000;
    }
    
    /* return the calculated tick */
    return tick;
}
RTM_EXPORT(rt_tick_from_millisecond);

/**
 * dump hex format data to console device
 *
 * @param name name for hex object, it will show on log header
 * @param buf hex buffer
 * @param size buffer size
 */
void at_print_raw_cmd(const char *name, const char *buf, size_t size)
{
#define __is_print(ch)       ((unsigned int)((ch) - ' ') < 127u - ' ')
#define WIDTH_SIZE           32

    size_t i, j;

    for (i = 0; i < size; i += WIDTH_SIZE)
    {
        rt_kprintf("[D/AT] %s: %04X-%04X: ", name, i, i + WIDTH_SIZE);
        for (j = 0; j < WIDTH_SIZE; j++)
        {
            if (i + j < size)
            {
                rt_kprintf("%02X ", buf[i + j]);
            }
            else
            {
                rt_kprintf("   ");
            }
            if ((j + 1) % 8 == 0)
            {
                rt_kprintf(" ");
            }
        }
        rt_kprintf("  ");
        for (j = 0; j < WIDTH_SIZE; j++)
        {
            if (i + j < size)
            {
                rt_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        rt_kprintf("\n");
    }
}

const char *at_get_last_cmd(size_t *cmd_size)
{
    *cmd_size = last_cmd_len;
    return send_buf;
}

size_t at_vprintf(   const char *format, va_list args)
{
    last_cmd_len = vsnprintf(send_buf, sizeof(send_buf), format, args);

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("sendline", send_buf, last_cmd_len);
#endif

    return 0;
//    return rt_device_write(device, 0, send_buf, last_cmd_len);
}

size_t at_vprintfln(  const char *format, va_list args)
{
    size_t len;

    len = at_vprintf( format, args);

  //  rt_device_write(device, 0, "\r\n", 2);

    return len + 2;
}
