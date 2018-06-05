/*
 * serial_test.c
 *
 *  Created on: 2016Äê7ÔÂ26ÈÕ
 *      Author: dongb
 */


#include <board.h>
#include <rtthread.h>
#include <debug.h>
#include <string.h>
#include "serial_test.h"
#include "motor_test.h"
#include "rtdevice.h"

#define SERIAL_NAME "uart1"

#define SERIAL_EVENT_RX		0x01

#define along  'w'
#define left   'a'
#define right  'd'
#define down   't'
#define back   's'

rt_device_t serial_dev = RT_NULL;
rt_event_t serial_event = RT_NULL;
rt_mailbox_t Serial_mb1 = RT_NULL;

rt_err_t serial_rx_indicate(rt_device_t dev, rt_size_t size)
{
	rt_event_send(serial_event, SERIAL_EVENT_RX);
	return RT_EOK;
}

rt_err_t serial_init(void)
{
	rt_err_t ret = -RT_ERROR;

	serial_event = rt_event_create("srl_t", RT_IPC_FLAG_FIFO);
	if(!serial_event)
	{
		debug_printf("Can't create serial_event");
		goto FAIL;
	}

	serial_dev = rt_device_find(SERIAL_NAME);
	if(!serial_dev)
	{
		debug_printf("Can't find %s", SERIAL_NAME);
		ret = -RT_ENOSYS;
		goto RELEASE_EVENT;
	}

	ret = rt_device_open(serial_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
	if(ret != RT_EOK)
	{
		debug_printf("Can't open %s, ret=%d", SERIAL_NAME, ret);
		ret = -RT_EIO;
		goto RELEASE_DEV;
	}

	rt_device_set_rx_indicate(serial_dev, serial_rx_indicate);

	return RT_EOK;

RELEASE_DEV:
	serial_dev = RT_NULL;
RELEASE_EVENT:
	rt_event_delete(serial_event);
FAIL:
	return ret;
}

void handle_rcdata(rt_uint8_t *buffer)
{
    rt_uint8_t i=0;
    for(i=0;buffer[i]!='\0';i++){
        switch (buffer[i]){
        case 'w':
            car_go(along);break;
        case 'a':
            car_go(left);break;
        case 'd':
            car_go(right);break;
        case 's':
            car_go(back);break;
        default:
            car_go(down);
        }
    }
    memset(buffer,'\0',64);
}

void handle_pi_data(rt_uint8_t *buffer)
{
//    if(buffer[0]>5)
//        return ;
    rt_uint16_t Orientation=650;
    rt_uint8_t i=0;
    rt_err_t ret;
    for(i=0;;i++){
        if(buffer[i]==0xfe){
            Orientation=buffer[i+2];
            Orientation=Orientation<<8;
            Orientation+=buffer[i+1];
//            debug_printf("Recv Ori=%d",Orientation);
            ret=rt_mb_send(Serial_mb1, Orientation);
//            debug_printf("send Ori ret=%d",ret);

        }
        if(buffer[i+3]!='\0'){
            break;
        }
    }
}
void serial_thread_entry(void *param)
{
	rt_err_t ret;
	rt_uint32_t event;
	rt_uint32_t count;

	static rt_uint8_t data[64];
	rt_size_t read_size;

	ret = serial_init();
	if(ret != RT_EOK)
	{
		return;
	}
	Serial_mb1 = rt_mb_create("Serial_mb1", 10,RT_IPC_FLAG_PRIO);
    if(!Serial_mb1)
    {
        debug_printf("Can't create Serial_mb1");
        return;
    }

	while(1)
	{

		// try to read data
		read_size = rt_device_read(serial_dev, 0, data, sizeof(data));
		// if read nothing, then wait the rx event
		if(read_size == 0)
		{
		    count++;
			// wait rx event
		    if(count==20){
                car_go(down);
                count=0;
                rt_event_recv(serial_event, SERIAL_EVENT_RX, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &event);
		    }
		    else{
//			rt_event_recv(serial_event, SERIAL_EVENT_RX, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &event);//
			rt_event_recv(serial_event, SERIAL_EVENT_RX, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, rt_tick_from_millisecond(10), &event);
			continue;
		    }
		}
		// send them back
//		handle_rcdata(data);
		handle_pi_data(data);
		rt_device_write(serial_dev, 0, data, read_size);
	}
}

void serial_test_init(void)
{
	rt_thread_t tid;
	tid = rt_thread_create("srl_t",
			serial_thread_entry, RT_NULL,
			RT_THREAD_STACK_SIZE_NORMAL, 26, RT_THREAD_TICK_NORMAL);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
}

