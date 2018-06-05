/*
 * dev_motor.h
 *
 *  Created on: 2016Äê7ÔÂ22ÈÕ
 *      Author: dbw
 */

#ifndef SRC_DRIVERS_DEV_MOTOR_H_
#define SRC_DRIVERS_DEV_MOTOR_H_

#include <stdint.h>
typedef enum
{
    MOTOR_LEFT = 0,
    MOTOR_RIGHT,
}motor_side_e;

void stop_now();
void rt_dev_motor_init(void);
void rt_dev_motor_set_speed(int left, int right);
void rt_dev_motor_get_location(int32_t *left, int32_t *right);
void rt_dev_motor_get_velocity(int32_t *left, int32_t *right);
void turn_to_orien(uint32_t ang,uint32_t ori);
void go_stright(uint32_t time);
void motor_set_throttle(motor_side_e side, int speed);

#endif /* SRC_DRIVERS_DEV_MOTOR_H_ */
