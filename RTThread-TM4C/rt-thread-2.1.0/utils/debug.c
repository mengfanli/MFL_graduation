/*
 * debug.c
 *
 *  Created on: Mar 23, 2016
 *      Author: Administrator
 */

#include <rtthread.h>
#include "debug.h"

void debug_print_array(const char *prefix, rt_uint8_t *buf, rt_size_t size)
{
//    if(!is_debug())
//    {
//        return;
//    }

    debug_printf("%s size:%u", prefix, size);
    for(rt_size_t i = 0; i < size; i++)
    {
        rt_kprintf("%02x ", buf[i]);
    }
    rt_kprintf("\n");
}
