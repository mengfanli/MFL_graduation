/*
 * maping.c
 *
 *  Created on: 2018Äê5ÔÂ15ÈÕ
 *      Author: mengfanli2
 */

#include <board.h>
#include <rtthread.h>
#include <debug.h>
#include "inc/hw_memmap.h"
#include <dev_hc_sr04.h>
#include "driverlib/sysctl.h"
#include "maping.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"


void maping_thread_entry(void *param)
{

}

void maping_init(void)
{
    rt_thread_t tid;
    tid = rt_thread_create("maping_t",
            maping_thread_entry, RT_NULL,
            RT_THREAD_STACK_SIZE_NORMAL, RT_THREAD_PRIORITY_NORMAL, RT_THREAD_TICK_NORMAL);
    if (tid != RT_NULL){
        rt_thread_startup(tid);
        debug_printf("success to crate maping_t");
    }
    else
        debug_printf("fail to crate maping_t");

}
