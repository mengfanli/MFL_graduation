/*
 * dev_hc_sr04.h
 *
 *  Created on: 2018Äê4ÔÂ14ÈÕ
 *      Author: mengfanli2
 */

#ifndef SRC_DRIVERS_DEV_HC_SR04_H_
#define SRC_DRIVERS_DEV_HC_SR04_H_

#include <rtthread.h>

#define trigger_port GPIO_PORTA_BASE
#define trigger_pin1 GPIO_PIN_4
#define trigger_pin2 GPIO_PIN_5


#define ECHO_EVENT_CHANNEL1 01
#define ECHO_EVENT_CHANNEL2 02
#define ECHO_EVENT_CHANNEL3 03

//extern rt_mailbox_t Usonic_mb;
//extern rt_mailbox_t Usonic_mb = RT_NULL;
void rt_dev_hc_sr04_init(void);
void Usonic_trigger(uint32_t trigger_pin);
void turn_sonic(uint8_t delta,uint16_t angle);
#endif /* SRC_DRIVERS_DEV_HC_SR04_H_ */
