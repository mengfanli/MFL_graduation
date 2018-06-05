/*
 * dev_key.h
 *
 *  Created on: 2016Äê7ÔÂ23ÈÕ
 *      Author: dbw
 */

#ifndef SRC_DRIVERS_DEV_KEY_H_
#define SRC_DRIVERS_DEV_KEY_H_

#include <rtthread.h>

/* this minimum interval of click event, unit: ms */
#define KEY_EVENT_CLICK_INTERVAL   50

#define KEY_EVENT_NONE          0x00
#define KEY_EVENT_SW1_CLICK     0x01
#define KEY_EVENT_SW2_CLICK     0x02

extern void rt_dev_key_init(void);
extern rt_uint32_t rt_dev_key_wait(rt_uint32_t event);
extern rt_uint32_t rt_dev_key_wait2(rt_uint32_t event, rt_uint32_t timeout);

#endif /* SRC_DRIVERS_DEV_KEY_H_ */
