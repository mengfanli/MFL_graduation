/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2014, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2014-07-18     ArdaFu       the first version for TM4C129X
 */

#include <rtthread.h>
#include <components.h>

#include "drv_i2c.h"
#include "dev_motor.h"
#include "dev_key.h"
#include "dev_hc_sr04.h"

#include "blinky.h"
#include "motor_test.h"
#include "test_1314.h"
#include "serial_test.h"
#include "Usonic.h"

void init_dev(void);
void init_app(void);

/* thread phase init */
void rt_init_thread_entry(void *parameter)
{
    /* Initialization RT-Thread Components */
//    rt_components_init();
	blinky_init();
	init_dev();
	init_app();

	rt_kprintf("\nApplication has initialized !!!\n");
}

void init_dev(void)
{
	rt_dev_key_init();
	rt_dev_motor_init();
	rt_dev_hc_sr04_init();
}

void init_app(void)
{
    motor_test_init();
    test1314_init();
    Usonic_init();
	serial_test_init();
}

int rt_application_init(void)
{
    rt_thread_t tid;
    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);

    return 0;
}

