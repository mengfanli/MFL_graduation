/*
 * dev_motor.c
 *
 *  Created on: 2016Äê7ÔÂ22ÈÕ
 *      Author: dbw
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "debug.h"
#include "board.h"

#include "dev_motor.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/pwm.h"
#include "driverlib/qei.h"
#include <stdlib.h>

/*----------------------------- Private define -------------------------------*/

#define MOTOR_NUM   2

/*
 * pin assignment
 *
 *-------------------------
 *  TM4C    |    L298N
 *-------------------------
 *  PB6     |   IN1
 *  PB7     |   IN2
 *  PB4     |   IN3
 *  PB5     |   IN4
 *-------------------------
 *7
 */

#define MOTOR_PWM_PERIPH    SYSCTL_PERIPH_PWM0
#define MOTOR_PWM_BASE      PWM0_BASE
#define MOTOR_PWM_GEN_0     PWM_GEN_0
#define MOTOR_PWM_GEN_1     PWM_GEN_1

#define MOTOR_PWM_OUT_LEFT_FRONT    PWM_OUT_0
#define MOTOR_PWM_OUT_LEFT_BACK     PWM_OUT_1
#define MOTOR_PWM_OUT_RIGHT_FRONT   PWM_OUT_2
#define MOTOR_PWM_OUT_RIGHT_BACK    PWM_OUT_3

#define MOTOR_PWM_PERIOD    400

#define MOTOR_PWM_GPIO_PERIPH       SYSCTL_PERIPH_GPIOB
#define MOTOR_PWM_GPIO_PORT         GPIO_PORTB_BASE
#define MOTOR_PWM_GPIO_PIN_CFG0     GPIO_PB6_M0PWM0
#define MOTOR_PWM_GPIO_PIN_CFG1     GPIO_PB7_M0PWM1
#define MOTOR_PWM_GPIO_PIN_CFG2     GPIO_PB4_M0PWM2
#define MOTOR_PWM_GPIO_PIN_CFG3     GPIO_PB5_M0PWM3
#define MOTOR_PWM_GPIO_PIN_NUM      (GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_4 | GPIO_PIN_5)
#define MOTOR_PWM_OUT_BIT           (PWM_OUT_0_BIT | PWM_OUT_1_BIT | PWM_OUT_2_BIT | PWM_OUT_3_BIT)

/*
 * pin assignment
 *
 *-------------------------
 *  TM4C    |    Encoder
 *-------------------------
 *  PD6     |   PhA0
 *  PD7     |   PhB0
 *  PC5     |   PhA1
 *  PC6     |   PhB1
 *-------------------------
 *
 */
#define MOTOR_QEI_PH0_GPIO_PERIPH  SYSCTL_PERIPH_GPIOD
#define MOTOR_QEI_PH0_GPIO_PORT    GPIO_PORTD_BASE
#define MOTOR_QEI_PH0_GPIO_PIN     (GPIO_PIN_6 | GPIO_PIN_7)
#define MOTOR_QEI_PH0_GPIO_CFG_A   GPIO_PD6_PHA0
#define MOTOR_QEI_PH0_GPIO_CFG_B   GPIO_PD7_PHB0
#define MOTOR_QEI_PH0_SWAP_FLAG    QEI_CONFIG_SWAP

#define MOTOR_QEI_PH1_GPIO_PERIPH  SYSCTL_PERIPH_GPIOC
#define MOTOR_QEI_PH1_GPIO_PORT    GPIO_PORTC_BASE
#define MOTOR_QEI_PH1_GPIO_PIN     (GPIO_PIN_5 | GPIO_PIN_6)
#define MOTOR_QEI_PH1_GPIO_CFG_A   GPIO_PC5_PHA1
#define MOTOR_QEI_PH1_GPIO_CFG_B   GPIO_PC6_PHB1
#define MOTOR_QEI_PH1_SWAP_FLAG    QEI_CONFIG_NO_SWAP

//
// frequence to calculate velocity
//
#define MOTOR_QEI_VEL_FREQ      10

//
// qei event
//
#define MOTOR_QEI_EVENT_NONE                        0x00
#define MOTOR_QEI_EVENT_VELOCITY_UPDATE_LEFT        0x01
#define MOTOR_QEI_EVENT_VELOCITY_UPDATE_RIGHT       0x02

//
//  velocity pid parameter
//
#define VEL_ADJ_I       0.25f
#define VEL_ADJ_I_MAX   400

/*----------------------------- Private typedef ------------------------------*/

//typedef enum
//{
//    MOTOR_LEFT = 0,
//    MOTOR_RIGHT,
//}motor_side_e;

typedef struct
{
    uint32_t periph;
    uint32_t base;
    uint32_t swap_flag;
    uint32_t interrupt;

    uint32_t gpio_periph;
    uint32_t gpio_port;
    uint8_t  gpio_pin;
    uint32_t gpio_cfg_a;
    uint32_t gpio_cfg_b;

    int32_t dir;
    int32_t velocity;   /*  unit: pulse edge per second */

}motor_qei_data_t;


/**
 * velocity pid
 */
typedef struct
{
    int32_t target;

    int32_t integral;
}vel_pid_t;

/*------------------------------ Private variables ---------------------------*/

motor_qei_data_t qei_data_array[MOTOR_NUM] =
{
        {
                .periph = SYSCTL_PERIPH_QEI0,
                .base = QEI0_BASE,
                .swap_flag = MOTOR_QEI_PH0_SWAP_FLAG,
                .interrupt = INT_QEI0,

                .gpio_periph = MOTOR_QEI_PH0_GPIO_PERIPH,
                .gpio_port = MOTOR_QEI_PH0_GPIO_PORT,
                .gpio_pin = MOTOR_QEI_PH0_GPIO_PIN,
                .gpio_cfg_a = MOTOR_QEI_PH0_GPIO_CFG_A,
                .gpio_cfg_b = MOTOR_QEI_PH0_GPIO_CFG_B,

                .dir = 1,
                .velocity = 0,
        },

        {
                .periph = SYSCTL_PERIPH_QEI1,
                .base = QEI1_BASE,
                .swap_flag = MOTOR_QEI_PH1_SWAP_FLAG,
                .interrupt = INT_QEI1,

                .gpio_periph = MOTOR_QEI_PH1_GPIO_PERIPH,
                .gpio_port = MOTOR_QEI_PH1_GPIO_PORT,
                .gpio_pin = MOTOR_QEI_PH1_GPIO_PIN,
                .gpio_cfg_a = MOTOR_QEI_PH1_GPIO_CFG_A,
                .gpio_cfg_b = MOTOR_QEI_PH1_GPIO_CFG_B,

                .dir = 1,
                .velocity = 0,
        },
};

vel_pid_t vel_pid_array[MOTOR_NUM];

rt_event_t motor_qei_event = RT_NULL;

/* ----------------------- Private function prototypes ---------------------*/

void motor_init_pwm(void);
void motor_init_qei(void);
void motor_init_qei2(motor_qei_data_t *qei_data);
//void motor_set_throttle(motor_side_e side, int speed);
void qei_irq_handler(motor_side_e side);

void vel_pid_reset(vel_pid_t *pid);
int32_t vel_pid_update(vel_pid_t *pid, int32_t current, float t_interval);

void turn_to_orien(uint32_t ang,uint32_t ori);

void motor_monitor_thread_entry(void *param);

/*-------------------------------- Functions -------------------------------*/

void stop_now()
{
    vel_pid_reset(vel_pid_array + 0);
    vel_pid_reset(vel_pid_array + 1);
}

void rt_dev_motor_init(void)
{
    motor_qei_event = rt_event_create("qei", RT_IPC_FLAG_FIFO);
    if(!motor_qei_event)
    {
        debug_printf("Can't create motor_qei_event");
        goto END;
    }
    vel_pid_reset(vel_pid_array + 0);
    vel_pid_reset(vel_pid_array + 1);

    rt_thread_t tid;
    tid = rt_thread_create("motor_mn",
            motor_monitor_thread_entry, RT_NULL,
            RT_THREAD_STACK_SIZE_NORMAL, RT_THREAD_PRIORITY_NORMAL, RT_THREAD_TICK_NORMAL);
    if (tid == RT_NULL)
    {
        debug_printf("Can't create motor_monitor_thread");
        goto END;
    }
    rt_thread_startup(tid);

    motor_init_pwm();
    motor_init_qei();

END:
    return;
}

void motor_init_pwm(void)
{
    SysCtlPWMClockSet(SYSCTL_PWMDIV_4);
    //
    // Enable the PWM0 peripheral
    //
    SysCtlPeripheralEnable(MOTOR_PWM_PERIPH);
    //
    // Wait for the PWM0 module to be ready.
    //
    while (!SysCtlPeripheralReady(MOTOR_PWM_PERIPH));

    //
    // Configure gpio
    //
    SysCtlPeripheralEnable(MOTOR_PWM_GPIO_PERIPH);
    GPIOPinConfigure(MOTOR_PWM_GPIO_PIN_CFG0);
    GPIOPinConfigure(MOTOR_PWM_GPIO_PIN_CFG1);
    GPIOPinConfigure(MOTOR_PWM_GPIO_PIN_CFG2);
    GPIOPinConfigure(MOTOR_PWM_GPIO_PIN_CFG3);
    GPIOPinTypePWM(MOTOR_PWM_GPIO_PORT, MOTOR_PWM_GPIO_PIN_NUM);

    //
    // Configure the PWM generator for count down mode with immediate updates
    // to the parameters.
    //
    PWMGenConfigure(MOTOR_PWM_BASE, MOTOR_PWM_GEN_0,
            PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(MOTOR_PWM_BASE, MOTOR_PWM_GEN_1,
            PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    //
    // Set the period. For a 50 KHz frequency, the period = 1/50,000, or 20
    // microseconds. For a 20 MHz clock, this translates to 400 clock ticks.
    // Use this value to set the period.
    //
    PWMGenPeriodSet(MOTOR_PWM_BASE, MOTOR_PWM_GEN_0, MOTOR_PWM_PERIOD);
    PWMGenPeriodSet(MOTOR_PWM_BASE, MOTOR_PWM_GEN_1, MOTOR_PWM_PERIOD);

    motor_set_throttle(MOTOR_LEFT, 0);
    motor_set_throttle(MOTOR_RIGHT, 0);

    //
    // Start the timers in generator 0.
    //
    PWMGenEnable(MOTOR_PWM_BASE, MOTOR_PWM_GEN_0);
    PWMGenEnable(MOTOR_PWM_BASE, MOTOR_PWM_GEN_1);
    //
    // Enable the outputs.
    //
    PWMOutputState(MOTOR_PWM_BASE, MOTOR_PWM_OUT_BIT, true);
}

void motor_init_qei(void)
{
    for(int i = 0; i < sizeof(qei_data_array) / sizeof(qei_data_array[0]); i++)
    {
        motor_init_qei2(qei_data_array + i);
    }
}

void motor_init_qei2(motor_qei_data_t *qei_data)
{
    //
    // Enable the QEI0 peripheral
    //
    SysCtlPeripheralEnable(qei_data->periph);
    //
    // Wait for the QEI0 module to be ready.
    //
    while(!SysCtlPeripheralReady(qei_data->periph));

    //
    // Configure gpio
    //
    SysCtlPeripheralEnable(qei_data->gpio_periph);
    while(!SysCtlPeripheralReady(qei_data->gpio_periph));
    if(qei_data->gpio_port == GPIO_PORTD_BASE && (qei_data->gpio_pin & GPIO_PIN_7))
    {
        //
        // unlock PF0
        //
        HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0x4C4F434B;
        HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= GPIO_PIN_7;
        HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0x00;
    }
    GPIOPinConfigure(qei_data->gpio_cfg_a);
    GPIOPinConfigure(qei_data->gpio_cfg_b);
    GPIOPinTypeQEI(qei_data->gpio_port, qei_data->gpio_pin);

    //
    // Configure the quadrature encoder to capture edges on both signals and
    // maintain an absolute position by resetting on index pulses. Using a
    // 1000 line encoder at four edges per line, there are 4000 pulses per
    // revolution; therefore set the maximum position to 3999 as the count
    // is zero based.
    //
    QEIConfigure(qei_data->base, (QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_NO_RESET | QEI_CONFIG_QUADRATURE | qei_data->swap_flag), 0xFFFFFFFF);
    //
    // Enable the quadrature encoder.
    //
    QEIEnable(qei_data->base);

    //
    // Configure qei velocity
    //
    QEIVelocityConfigure(qei_data->base, QEI_VELDIV_1, SysCtlClockGet() / MOTOR_QEI_VEL_FREQ);
    QEIVelocityEnable(qei_data->base);

    QEIIntEnable(qei_data->base, QEI_INTDIR | QEI_INTTIMER);

    IntEnable(qei_data->interrupt);
}

void rt_dev_motor_set_speed(int left, int right)
{
//    motor_set_throttle(MOTOR_LEFT, left);
//    motor_set_throttle(MOTOR_RIGHT, right);

    vel_pid_array[MOTOR_LEFT].target = left;
    vel_pid_array[MOTOR_RIGHT].target = right;
}

void motor_set_throttle(motor_side_e side, int speed)
{
    rt_int32_t pulse_width = speed * MOTOR_PWM_PERIOD / 100;

    if(pulse_width < 0)
    {
        pulse_width = -pulse_width;
    }
    else if(pulse_width == 0)
    {
        // Can't be zero
        pulse_width = 1;
    }

    if(side == MOTOR_LEFT)
    {
        if(speed >= 0)
        {
            PWMPulseWidthSet(MOTOR_PWM_BASE, MOTOR_PWM_OUT_LEFT_FRONT, (rt_uint32_t)pulse_width);
            PWMPulseWidthSet(MOTOR_PWM_BASE, MOTOR_PWM_OUT_LEFT_BACK, 1);
        }
        else
        {
            PWMPulseWidthSet(MOTOR_PWM_BASE, MOTOR_PWM_OUT_LEFT_FRONT, 1);
            PWMPulseWidthSet(MOTOR_PWM_BASE, MOTOR_PWM_OUT_LEFT_BACK, (rt_uint32_t)pulse_width);
        }
    }
    else
    {
        if(speed >= 0)
        {
            PWMPulseWidthSet(MOTOR_PWM_BASE, MOTOR_PWM_OUT_RIGHT_FRONT, (rt_uint32_t)pulse_width);
            PWMPulseWidthSet(MOTOR_PWM_BASE, MOTOR_PWM_OUT_RIGHT_BACK, 1);
        }
        else
        {
            PWMPulseWidthSet(MOTOR_PWM_BASE, MOTOR_PWM_OUT_RIGHT_FRONT, 1);
            PWMPulseWidthSet(MOTOR_PWM_BASE, MOTOR_PWM_OUT_RIGHT_BACK, (rt_uint32_t)pulse_width);
        }
    }
}

void rt_dev_motor_get_location(int32_t *left, int32_t *right)
{
    *left =  (int32_t)QEIPositionGet(qei_data_array[MOTOR_LEFT].base);
    *right = (int32_t)QEIPositionGet(qei_data_array[MOTOR_RIGHT].base);
}

void rt_dev_motor_get_velocity(int32_t *left, int32_t *right)
{
    *left = qei_data_array[MOTOR_LEFT].velocity;
    *right = qei_data_array[MOTOR_RIGHT].velocity;
}

void QEI0_IRQHandler(void)
{
    qei_irq_handler(MOTOR_LEFT);
}

void QEI1_IRQHandler(void)
{
    qei_irq_handler(MOTOR_RIGHT);
}

void qei_irq_handler(motor_side_e side)
{
    uint32_t status = QEIIntStatus(qei_data_array[side].base, true);
    QEIIntClear(qei_data_array[side].base, status);

    if(status & QEI_INTINDEX)
    {
//        debug_printf("%s int_index", side == MOTOR_LEFT ? "left" : "right");
    }

    if(status & QEI_INTTIMER)
    {
//        debug_printf("%s int_timer", side == MOTOR_LEFT ? "left" : "right");
        rt_event_send(motor_qei_event, side == MOTOR_LEFT ?
                MOTOR_QEI_EVENT_VELOCITY_UPDATE_LEFT : MOTOR_QEI_EVENT_VELOCITY_UPDATE_RIGHT);
    }

    if(status & QEI_INTDIR)
    {
        qei_data_array[side].dir = -qei_data_array[side].dir;
    }
}

void vel_pid_reset(vel_pid_t *pid)
{
    rt_memset(pid, 0, sizeof(*pid));
}

int32_t vel_pid_update(vel_pid_t *pid, int32_t current, float t_interval)
{
    float output_tmp;
    int32_t output;
    int32_t error;

    error = pid->target - current;

    pid->integral += error * t_interval;
    pid->integral = pid->integral > VEL_ADJ_I_MAX ? VEL_ADJ_I_MAX : pid->integral;
    pid->integral = pid->integral < -VEL_ADJ_I_MAX ? -VEL_ADJ_I_MAX : pid->integral;

    output_tmp = 0;
    output_tmp += pid->integral * VEL_ADJ_I;
    output = output_tmp;

    output = output < -100 ? -100 : output;
    output = output > 100 ? 100 : output;

    return output;
}

void turn_to_orien(uint32_t ang,uint32_t ori)
{
    uint32_t err=abs(ang-ori);
    int throt=ang>ori?-(ang-ori):(ori-ang);
    motor_set_throttle(MOTOR_LEFT,throt);
    motor_set_throttle(MOTOR_RIGHT,-throt);
    rt_thread_delay(err);
    motor_set_throttle(MOTOR_LEFT,0);
    motor_set_throttle(MOTOR_RIGHT,0);
}
void go_stright(uint32_t time)
{
    stop_now();
    uint8_t throt=200;
    rt_dev_motor_set_speed(throt, throt);
    rt_thread_delay(time);
    rt_dev_motor_set_speed(0, 0);

}

void motor_monitor_thread_entry(void *param)
{
    rt_int32_t output;
    rt_uint32_t event;
    rt_bool_t update_flag[MOTOR_NUM];

    while(1)
    {
        rt_event_recv(motor_qei_event, MOTOR_QEI_EVENT_VELOCITY_UPDATE_LEFT | MOTOR_QEI_EVENT_VELOCITY_UPDATE_RIGHT,
                RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &event);

        update_flag[MOTOR_LEFT] = update_flag[MOTOR_RIGHT] = RT_FALSE;

        if(event & MOTOR_QEI_EVENT_VELOCITY_UPDATE_LEFT)
        {
            update_flag[MOTOR_LEFT] = RT_TRUE;
        }
        if(event & MOTOR_QEI_EVENT_VELOCITY_UPDATE_RIGHT)
        {
            update_flag[MOTOR_RIGHT] = RT_TRUE;
        }

        for(int index = 0; index < MOTOR_NUM; index++)
        {
            if(!update_flag[index])
            {
                continue;
            }

            qei_data_array[index].velocity = QEIVelocityGet(qei_data_array[index].base) * MOTOR_QEI_VEL_FREQ * QEIDirectionGet(qei_data_array[index].base);

            output = vel_pid_update(vel_pid_array + index, qei_data_array[index].velocity, 0.2);
            motor_set_throttle(index, output);
//            debug_printf("%5s vel:%04d thr:%04d", index == MOTOR_LEFT ? "left" : "right",
//                    qei_data_array[index].velocity, output);
        }

    }
}
