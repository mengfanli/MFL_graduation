/*
 * dev_key.c
 *
 *  Created on: 2016Äê7ÔÂ23ÈÕ
 *      Author: dbw
 */

#include <rtthread.h>
#include "board.h"
#include "dev_key.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"

#include "debug.h"

/*
 * pin assignment
 *
 *-------------------------
 *  TM4C    |    Key
 *-------------------------
 *  PF4     |   SW1
 *  PF0     |   SW2
 *-------------------------
 *
 */

#define KEY_SW1_INDEX       0
#define KEY_SW1_GPIO_PERIPH SYSCTL_PERIPH_GPIOF
#define KEY_SW1_GPIO_PORT   GPIO_PORTF_BASE
#define KEY_SW1_GPIO_PIN    GPIO_PIN_4
#define KEY_SW1_GPIO_INT    INT_GPIOF

#define KEY_SW2_INDEX       1
#define KEY_SW2_GPIO_PERIPH SYSCTL_PERIPH_GPIOF
#define KEY_SW2_GPIO_PORT   GPIO_PORTF_BASE
#define KEY_SW2_GPIO_PIN    GPIO_PIN_0
#define KEY_SW2_GPIO_INT    INT_GPIOF

#define KEY_INNER_EVENT_SW1_MISC     0x01
#define KEY_INNER_EVENT_SW2_MISC     0x02

typedef struct
{
    uint32_t periph;
    uint32_t port;
    uint8_t  pin;
    uint32_t interrupt;

    uint32_t last_fall_time;
    uint32_t last_rise_time;

    uint32_t usr_event;
}key_data_t;

rt_event_t key_inner_event = RT_NULL;
rt_event_t key_event = RT_NULL;

key_data_t key_data_array[] =
{
        {
                .periph = KEY_SW1_GPIO_PERIPH,
                .port = KEY_SW1_GPIO_PORT,
                .pin = KEY_SW1_GPIO_PIN,
                .interrupt = KEY_SW1_GPIO_INT,
                .last_fall_time = 0,
                .last_rise_time = 0,
                .usr_event = KEY_EVENT_SW1_CLICK,
        },

        {
                .periph = KEY_SW2_GPIO_PERIPH,
                .port = KEY_SW2_GPIO_PORT,
                .pin = KEY_SW2_GPIO_PIN,
                .interrupt = KEY_SW2_GPIO_INT,
                .last_fall_time = 0,
                .last_rise_time = 0,
                .usr_event = KEY_EVENT_SW2_CLICK,
        },
};

void key_init_hw(uint32_t key_periph, uint32_t key_port, uint8_t key_pin, uint32_t key_int);
void key_thread_entry(void *param);
void key_handler_inner_event(int index);

void GPIOF_IRQHandler(void)
{
    uint32_t status;

    /* enter interrupt */
    rt_interrupt_enter();

    status = GPIOIntStatus(GPIO_PORTF_BASE, true);
    GPIOIntClear(GPIO_PORTF_BASE, status);

    if(status & KEY_SW1_GPIO_PIN)
    {
        rt_event_send(key_inner_event, KEY_INNER_EVENT_SW1_MISC);
    }
    if(status & KEY_SW2_GPIO_PIN)
    {
        rt_event_send(key_inner_event, KEY_INNER_EVENT_SW2_MISC);
    }

    /* leave interrupt */
    rt_interrupt_leave();
}

void rt_dev_key_init(void)
{
    rt_thread_t tid;

    key_inner_event = rt_event_create("key_in", RT_IPC_FLAG_FIFO);
    if(!key_inner_event)
    {
        debug_printf("Can't create key_inner_event");
        goto END;
    }

    key_event = rt_event_create("key", RT_IPC_FLAG_FIFO);
    if(!key_event)
    {
        debug_printf("Can't create key_event");
        goto END;
    }

    tid = rt_thread_create("key",
            key_thread_entry, RT_NULL,
            RT_THREAD_STACK_SIZE_NORMAL, RT_THREAD_PRIORITY_NORMAL, RT_THREAD_TICK_NORMAL);
    if (!tid)
    {
        goto END;
    }
    rt_thread_startup(tid);

    for(int i = 0; i < sizeof(key_data_array) / sizeof(key_data_array[0]); i++)
    {
        key_data_t *key_data = key_data_array + i;
        key_init_hw(key_data->periph, key_data->port, key_data->pin, key_data->interrupt);
    }

END:
    return;
}

rt_uint32_t rt_dev_key_wait(rt_uint32_t event)
{
    return rt_dev_key_wait2(event, RT_WAITING_FOREVER);
}

rt_uint32_t rt_dev_key_wait2(rt_uint32_t event, rt_uint32_t timeout)
{
    rt_tick_t timeout_tick;
    rt_uint32_t event_recv;
    rt_err_t ret;

    if(timeout != RT_WAITING_FOREVER)
    {
        timeout_tick = rt_tick_from_millisecond(timeout);
    }
    else
    {
        timeout_tick = RT_WAITING_FOREVER;
    }

    ret = rt_event_recv(key_event, event,
            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, timeout_tick, &event_recv);
    if(ret != RT_EOK)
    {
        return KEY_EVENT_NONE;
    }
    else
    {
        return event_recv;
    }
}

void key_init_hw(uint32_t key_periph, uint32_t key_port, uint8_t key_pin, uint32_t key_int)
{
    //
    // Enable the GPIO peripheral
    //
    SysCtlPeripheralEnable(key_periph);
    //
    // Wait for the GPIO module to be ready.
    //
    while(!SysCtlPeripheralReady(key_periph));

    if(key_port == GPIO_PORTF_BASE && key_pin == GPIO_PIN_0)
    {
        //
        // unlock PF0
        //
        HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0x4C4F434B;
        HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= GPIO_PIN_0;
        HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0x00;
    }

    //
    // Initialize the GPIO pin configuration.
    //
    //
//    GPIOPinTypeGPIOInput(key_port, key_pin);
    GPIODirModeSet(key_port, key_pin, GPIO_DIR_MODE_IN);
    //
    // Set the pad(s) for standard push-pull operation.
    //
    GPIOPadConfigSet(key_port, key_pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    //
    // Make pins both edge triggered interrupts.
    //
    GPIOIntTypeSet(key_port, key_pin, GPIO_BOTH_EDGES);
    //
    // Enable the pin interrupts.
    //
    GPIOIntEnable(key_port, key_pin);
    IntEnable(key_int);
}

void key_thread_entry(void *param)
{
    rt_uint32_t event;
    while(1)
    {
        rt_event_recv(key_inner_event, KEY_INNER_EVENT_SW1_MISC | KEY_INNER_EVENT_SW2_MISC,
                RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &event);

        if(event & KEY_INNER_EVENT_SW1_MISC)
        {
//            debug_printf("recv sw1 event");
            key_handler_inner_event(KEY_SW1_INDEX);
        }
        if(event & KEY_INNER_EVENT_SW2_MISC)
        {
//            debug_printf("recv sw2 event");
            key_handler_inner_event(KEY_SW2_INDEX);
        }
    }

}

void key_handler_inner_event(int index)
{
    key_data_t *key_data = key_data_array + index;
    rt_uint32_t cur_time = rt_millis_get();

    /* falling event (press) */
    if(!GPIOPinRead(key_data->port, key_data->pin))
    {
        // button is shaking
        if(cur_time < key_data->last_rise_time + KEY_EVENT_CLICK_INTERVAL)
        {
            return;
        }
        key_data->last_fall_time = cur_time;
    }
    else
    {
        // hasn't been pressed
        if(key_data->last_fall_time < key_data->last_rise_time)
        {
            return;
        }
        key_data->last_rise_time = cur_time;
        rt_event_send(key_event, key_data->usr_event);
//        debug_printf("send key_event");
    }
}
