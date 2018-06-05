/*
 * debug.h
 *
 *  Created on: Feb 28, 2016
 *      Author: Administrator
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <rtthread.h>

#define debug_printf(fmt, ...)\
    {\
        if(1)\
        {\
        	rt_kprintf("[%s:%d] "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__);\
        }\
    }

extern void debug_print_array(const char *prefix, rt_uint8_t *buf, rt_size_t size);

#endif /* DEBUG_H_ */
