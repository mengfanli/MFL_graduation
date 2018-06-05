/*
 * motor_test.c
 *
 *  Created on: 2016Äê7ÔÂ23ÈÕ
 *      Author: dbw
 */

#include <board.h>
#include <rtthread.h>
#include <debug.h>

#include <dev_motor.h>
#include <dev_key.h>
#include <serial_test.h>
#include <test_1314.h>
#include "motor_test.h"


extern rt_mailbox_t Angle_mb2 ;
extern rt_mailbox_t Serial_mb1 ;


void car_go(uint8_t sig)
{
    int v_target=200;
    switch (sig){

    case 'w':
        rt_dev_motor_set_speed(v_target, v_target);break;
    case 'a':
        rt_dev_motor_set_speed(-v_target, v_target);break;
    case 'd':
        rt_dev_motor_set_speed(v_target, -v_target);break;
    case 's':
        rt_dev_motor_set_speed(-v_target, -v_target);break;
    default:
        rt_dev_motor_set_speed(0, 0);
    }
}

void Orientation_thread_entry(void *param)
{
    uint32_t orien=1000,angle=0;
    rt_err_t ret;
    debug_printf("wait to run_start");
    rt_dev_key_wait(KEY_EVENT_SW1_CLICK);

    debug_printf("wait to stop");
    while(1)
    {
        ret = rt_mb_recv(Serial_mb1,&orien,2);
        if(ret != RT_EOK)
        {
            debug_printf("fail to recv Smb1");
        }
        else
        {
            debug_printf("orientation = %d",orien);
            while(1)
            {
                ret = rt_mb_recv(Angle_mb2,&angle,2);
                if(ret != RT_EOK){
                    debug_printf("fail to recv ang1");
                }
                else{
                    debug_printf("ang1 = %d",angle);
                    uint32_t err=angle>orien? angle-orien:orien-angle;
                    if(err<13)
                        break;
                    int throt=angle>orien?120:-120;
                    rt_dev_motor_set_speed(throt, -throt);
                }
            }
            go_stright(250);
        }
    }
}
void motor_thread_entry(void *param)
{
    int v_target=200;
    while(1)
    {
        debug_printf("wait to run");
        rt_dev_key_wait(KEY_EVENT_SW1_CLICK);

        debug_printf("wait to stop");

        while(1)
        {
            rt_uint32_t event = rt_dev_key_wait2(KEY_EVENT_SW1_CLICK | KEY_EVENT_SW2_CLICK, 100);
            if(event == KEY_EVENT_NONE)
            {
            }
            else if(event == KEY_EVENT_SW1_CLICK)
            {
                break;
            }
            else if(event == KEY_EVENT_SW2_CLICK)
            {
                v_target = -v_target;
            }

            rt_dev_motor_set_speed(v_target, v_target);

//            rt_int32_t loc_left, loc_right;
//            rt_int32_t v_left, v_right;
//            rt_dev_motor_get_location(&loc_left, &loc_right);
//            rt_dev_motor_get_velocity(&v_left, &v_right);
//
//            debug_printf("current location:(%04d, %04d), velocity:(%04d,%04d)",
//                    loc_left, loc_right, v_left, v_right);
        }

        rt_dev_motor_set_speed(0, 0);
    }

}

void motor_test_init(void)
{
    rt_thread_t tid;
    tid = rt_thread_create("motor_ts",
//            motor_thread_entry, RT_NULL,
            Orientation_thread_entry, RT_NULL,
            RT_THREAD_STACK_SIZE_NORMAL, 27, RT_THREAD_TICK_NORMAL);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

}
