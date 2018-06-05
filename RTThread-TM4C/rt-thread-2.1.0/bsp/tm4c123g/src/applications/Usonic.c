/*
 * Usonic.c
 *
 *  Created on: 2018Äê4ÔÂ14ÈÕ
 *      Author: mengfanli2
 */
#include <board.h>
#include <rtthread.h>
#include <debug.h>
#include "inc/hw_memmap.h"
#include <dev_hc_sr04.h>
#include "driverlib/sysctl.h"
#include "Usonic.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

extern rt_mailbox_t Usonic_mb1 ;
extern rt_mailbox_t Usonic_mb2 ;
extern rt_mailbox_t Angle_mb1 ;
extern rt_device_t serial_dev;

#ifndef trigger_pin1
#define trigger_pin1 GPIO_PIN_2
#define trigger_pin2 GPIO_PIN_3
#endif

uint8_t angle=0;
uint8_t delta=10;
void Triger_thread_entry(void *param)
{
    uint8_t direct=1;
    while(1)
    {
        turn_sonic(delta,direct==1?++angle:--angle);
        if(angle==180/delta-1)
        {
            direct=0;
        }
        if(angle==0)
        {
            direct=1;
        }
//        debug_printf("ang1e=%d",angle);
        rt_thread_delay(10);
    }
}
void Usonic_thread_entry(void *param)
{
    rt_err_t ret;
    uint32_t travel_t;
    uint32_t distance1,distance2,car_ori=0;
    uint8_t need_send=1;
    uint16_t surround[360/delta];
//    uint16_t back_d[180/delta];
    uint8_t transtart=0xef;
    uint8_t transend=0xfe;
    uint8_t index;
    while(1)
    {
        rt_thread_delay(3);
        ret = rt_mb_recv(Angle_mb1,&car_ori,2);
        if(ret != RT_EOK){
//            debug_printf("fail to recv ang1");
        }
        else{
//        debug_printf("car_ori = %d",car_ori);

        }
        Usonic_trigger(trigger_pin1);
        ret = rt_mb_recv(Usonic_mb1,&travel_t,4);
        if(ret != RT_EOK){
//            debug_printf("fail to recv mb1");
        }
        else{
        distance1= (uint32_t)(travel_t*170/80000);
        surround[(angle+(car_ori*180)/(delta*314))%(360/delta)]=distance1;
//        debug_printf("Distance1 = %d",distance1);
        }
        rt_thread_delay(3);
        Usonic_trigger(trigger_pin2);
        ret = rt_mb_recv(Usonic_mb2,&travel_t,4);
        if(ret != RT_EOK){
//            debug_printf("fail to recv mb2");
        }
        else{
        distance2= (uint32_t)(travel_t*170/80000);
        surround[(180/delta+angle+(car_ori*180)/(delta*314))%(360/delta)]=distance2;
//        debug_printf("Distance2 = %d",distance2);
        }
        if(((angle==180/delta-1)||(angle==0)) && need_send){
            rt_device_write(serial_dev, 0, &transtart,1);
            rt_device_write(serial_dev, 0, surround, 2*360/delta);
            rt_device_write(serial_dev, 0, &transend,1);
            need_send=0;
            for(index=0;index<360/delta;index++){
            debug_printf("surr %d = %d",index,surround[index]);
            }
        }
        if(angle==3)
            need_send=1;
    }
}

void Usonic_init(void)
{
    rt_thread_t tid;
    tid = rt_thread_create("Usonic_t",
            Usonic_thread_entry, RT_NULL,
            RT_THREAD_STACK_SIZE_NORMAL, RT_THREAD_PRIORITY_NORMAL, RT_THREAD_TICK_NORMAL);
    if (tid != RT_NULL){
        rt_thread_startup(tid);
        debug_printf("success to crate Usonic_t");
    }
    else
        debug_printf("fail to crate Usonic_t");

    tid = rt_thread_create("Triger_t",
                Triger_thread_entry, RT_NULL,
                RT_THREAD_STACK_SIZE_NORMAL, RT_THREAD_PRIORITY_NORMAL, RT_THREAD_TICK_NORMAL);
    if (tid != RT_NULL){
        rt_thread_startup(tid);
        debug_printf("success to crate Triger_t");
    }
    else
        debug_printf("fail to crate Triger_t");
}
