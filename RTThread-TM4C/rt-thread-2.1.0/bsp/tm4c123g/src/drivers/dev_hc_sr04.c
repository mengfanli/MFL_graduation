/*
 * dev_hc_sr04.c
 *
 *  Created on: 2018年4月14日
 *      Author: mengfanli2
 */

#include <rtthread.h>
#include "board.h"
#include "dev_key.h"

#include <debug.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"

//#include "debug.h"

#include "dev_hc_sr04.h"
/*
 * pin assignment
 *
 *-------------------------
 *  TM4C    |    HC_SR04
 *-------------------------
 *  PA4     |   trigger1
 *  PA5     |   trigger2
 *  PB3     |   pwm_actuator
 *-------------------------
 *  PD0     |   echo1
 *  PD1     |   echo2
 *  PD2     |   echo3
 *
 */

rt_mailbox_t Usonic_mb1 = RT_NULL;
rt_mailbox_t Usonic_mb2 = RT_NULL;
rt_mailbox_t Usonic_mb3 = RT_NULL;

void triggerpins_init (void);
void timer_init();


void rt_dev_hc_sr04_init(void)
{
    Usonic_mb1 = rt_mb_create("Usonic1", 1,RT_IPC_FLAG_FIFO);
    if(!Usonic_mb1)
    {
        debug_printf("Can't create Usonic_mb1");
        goto END;
    }
    Usonic_mb2 = rt_mb_create("Usonic2", 1,RT_IPC_FLAG_FIFO);
    if(!Usonic_mb2)
    {
        debug_printf("Can't create Usonic_mb1");
        goto END;
    }

    Usonic_mb3 = rt_mb_create("Usonic3", 1,RT_IPC_FLAG_FIFO);
    if(!Usonic_mb3)
    {
        debug_printf("Can't create Usonic_mb1");
        goto END;
    }


    timer_init();
    triggerpins_init();
    debug_printf("After hc_sr04 dev init");

    END:
        return;
}
void turn_sonic(uint8_t delta,uint16_t angle)
{
    static uint32_t pause=0;
    pause = delta*angle*873+39321;
    TimerPrescaleMatchSet(TIMER3_BASE,TIMER_B,pause>>16);
    TimerMatchSet(TIMER3_BASE,  TIMER_B, delta*angle*873+39321);//设置匹配值
}

void Usonic_trigger(uint32_t trigger_pin)
{
    GPIOPinWrite(trigger_port,trigger_pin, trigger_pin);
//    GPIOPinWrite(GPIO_PORTF_BASE,trigger_pin, trigger_pin);
    SysCtlDelay(54);
    GPIOPinWrite(trigger_port,trigger_pin, 0);
//    GPIOPinWrite(GPIO_PORTF_BASE,trigger_pin, 0);
}
/*
 * Actually it is trigger pins init
 */
void triggerpins_init (void)
{
//    SysCtlPeripheralEnable(trigger_port1);
//    GPIOPinTypeGPIOOutput(trigger_port1, trigger_pin);
    SysCtlPeripheralEnable(trigger_port);
    GPIOPinTypeGPIOOutput(trigger_port, trigger_pin1|trigger_pin2);
//    SysCtlPeripheralEnable(GPIO_PORTC_BASE);
//    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, trigger_pin3);

//    GPIOPadConfigSet(trigger_port1, trigger_pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPadConfigSet(trigger_port, trigger_pin1|trigger_pin2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//    GPIOPadConfigSet(GPIO_PORTC_BASE, trigger_pin3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinWrite(trigger_port,trigger_pin1, 0);
    GPIOPinWrite(trigger_port,trigger_pin2, 0);
//    GPIOPinWrite(GPIO_PORTC_BASE,trigger_pin3, 0);
    //
}

void TimerIntHandler1(void)
{
    uint32_t time=0;

    /* enter interrupt */
    rt_interrupt_enter();
    // Clear the timer interrupt.
    //
    // TODO: Rework this for the timer you are using in your application.
    //

    TimerIntClear(WTIMER2_BASE,TIMER_CAPA_EVENT );
//    debug_printf("IN Wtimer2 A int");
    time=0xFFFFFFFF-TimerValueGet(WTIMER2_BASE, TIMER_A);

        if(time>20000 && time<2000000){
            rt_mb_send(Usonic_mb1, time);
        }


    TimerLoadSet(WTIMER2_BASE, TIMER_A, 0xFFFFFFFF);
    /* leave interrupt */
    rt_interrupt_leave();

}
void
TimerIntHandler2(void)
{
    uint32_t time=0;

    /* enter interrupt */
    rt_interrupt_enter();
    // Clear the timer interrupt.
    //
    // TODO: Rework this for the timer you are using in your application.
    //
    debug_printf("IN Wtimer2 B int");
    TimerIntClear(WTIMER2_BASE,TIMER_CAPB_EVENT );

    time=0xFFFFFFFF-TimerValueGet(WTIMER2_BASE, TIMER_B);

        if(time>6000 && time<2000000){
            rt_mb_send(Usonic_mb2, time);
        }


    TimerLoadSet(WTIMER2_BASE, TIMER_B, 0xFFFFFFFF);
    /* leave interrupt */
    rt_interrupt_leave();

}
void TimerIntHandler3(void)
{
    uint32_t time=0;

    /* enter interrupt */
    rt_interrupt_enter();
    // Clear the timer interrupt.
    //
    // TODO: Rework this for the timer you are using in your application.
    //
//    debug_printf("IN timer2 A int");
    TimerIntClear(WTIMER3_BASE,TIMER_CAPA_EVENT );

    time=0xFFFFFFFF-TimerValueGet(WTIMER3_BASE, TIMER_A);

        if(time>6000 && time<2000000){
            rt_mb_send(Usonic_mb2, time);
        }


    TimerLoadSet(WTIMER3_BASE, TIMER_A, 0xFFFFFFFF);
    /* leave interrupt */
    rt_interrupt_leave();

}
/*
 * The echo pins init
 */
void timer_init()
{
    ///
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPinConfigure(GPIO_PD0_WT2CCP0);
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_0,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    TimerConfigure(WTIMER2_BASE, (TIMER_CFG_SPLIT_PAIR |TIMER_CFG_A_CAP_TIME));

    TimerControlEvent(WTIMER2_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);

    TimerLoadSet(WTIMER2_BASE, TIMER_A, 0xFFFFFFFF);

    IntEnable(INT_WTIMER2A);


    TimerIntRegister(WTIMER2_BASE,TIMER_A,TimerIntHandler1);

    TimerIntEnable(WTIMER2_BASE, TIMER_CAPA_EVENT);
    TimerEnable(WTIMER2_BASE, TIMER_A);

/*
 *
 */

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);//使能外设PB
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);//使能Timer3

    //将PF1设置为CCP功能引脚，用于Timer0
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_3);
    GPIOPinConfigure(GPIO_PB3_T3CCP1);


    //设置Timer0的TimerB位PWM功能
    TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR |TIMER_CFG_B_PWM);

    TimerControlLevel(TIMER3_BASE, TIMER_B, true);

//    uint32_t ui32Period = 0; //50hz 20ms

    TimerPrescaleSet(TIMER3_BASE,TIMER_B,0x18);
    TimerLoadSet(TIMER3_BASE,  TIMER_B, 0 );//设置初值为10000

    TimerPrescaleMatchSet(TIMER3_BASE,TIMER_B,0);
    TimerMatchSet(TIMER3_BASE,  TIMER_B, 0x9999);//设置匹配值
    TimerEnable(TIMER3_BASE,  TIMER_B);//使能Timer0的TimerB
//    debug_printf("SysCtlClock 1ms = %d", ui32Period/20);//  80000=1ms

//  /*
//   *
//   */
//
//    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_1);
//    GPIOPinConfigure(GPIO_PD1_WT2CCP1);
//    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_1,
//                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//
//    TimerConfigure(WTIMER2_BASE, (TIMER_CFG_SPLIT_PAIR |TIMER_CFG_B_CAP_TIME|TIMER_CFG_A_CAP_TIME));
//
//    TimerControlEvent(WTIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
//
//
//    TimerLoadSet(WTIMER2_BASE, TIMER_B, 0xFFFFFFFF);
//
//    IntEnable(INT_WTIMER2B);
//    TimerIntRegister(WTIMER2_BASE,TIMER_B,TimerIntHandler2);
//
//    TimerIntEnable(WTIMER2_BASE, TIMER_CAPB_EVENT);
//    TimerEnable(WTIMER2_BASE, TIMER_B);
//    /*
//     *
//     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3);
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_2);
    GPIOPinConfigure(GPIO_PD2_WT3CCP0);
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
//
    TimerConfigure(WTIMER3_BASE, (TIMER_CFG_SPLIT_PAIR |TIMER_CFG_A_CAP_TIME));

    TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);

    TimerLoadSet(WTIMER3_BASE, TIMER_A, 0xFFFFFFFF);

    IntEnable(INT_WTIMER3A);

    TimerIntRegister(WTIMER3_BASE,TIMER_A,TimerIntHandler3);

    TimerIntEnable(WTIMER3_BASE, TIMER_CAPA_EVENT);
    TimerEnable(WTIMER3_BASE, TIMER_A);
}
