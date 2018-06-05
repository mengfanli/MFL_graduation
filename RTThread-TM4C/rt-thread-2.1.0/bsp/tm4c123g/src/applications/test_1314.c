/*
 * test_1314.c
 *
 *  Created on: 2016Äê7ÔÂ24ÈÕ
 *      Author: dbw
 */

#include "test_1314.h"
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <board.h>
#include <rtthread.h>
#include <debug.h>
#include <drv_i2c.h>

//#define SLAVE_ADDR  0x1e
#define SLAVE_ADDR  0x0d

rt_mailbox_t Angle_mb1 = RT_NULL;
rt_mailbox_t Angle_mb2 = RT_NULL;

static int32_t read_reg(uint8_t reg_addr)
{
	rt_err_t ret;
	rt_uint8_t tmp_reg_value[2];

	ret = rt_hw_i2c_read_reg(RT_I2C_1, SLAVE_ADDR, reg_addr, tmp_reg_value, 2);
	if(ret != RT_EOK)
	{
		debug_printf("read reg(0x%x) fail", reg_addr);
		return -1;
	}
//	return (tmp_reg_value[1] )| (tmp_reg_value[0] << 8);//hmc
	return (tmp_reg_value[1] ) << 8| (tmp_reg_value[0]);//Qmc
}

void test1314_thread_entry(void *param)
{
//	rt_uint8_t addr = 0x1e;
    rt_uint8_t addr = 0x0d;
	rt_err_t ret;
	rt_uint8_t tmp_reg_value;
	int32_t XH,YH,ZH;
	float angle;
	int angle_int;
//	rt_int32_t reg_value;

//	const rt_uint8_t cfg[][2] =
//	{
//	 {0x02, 0X00},
//	};
    const rt_uint8_t cfg[][2] =
    {
     {0x09, 0X01},
    };

	for(int i = 0; i < sizeof(cfg) / sizeof(cfg[0]); i++)
	{
		ret = rt_hw_i2c_write_reg(RT_I2C_1, addr, cfg[i][0], (rt_uint8_t *)(cfg[i] + 1), 1);
		if(ret != RT_EOK)
		{
			debug_printf("write reg(0x%x) fail", cfg[i][0]);
			goto ERROR;
		}

		ret = rt_hw_i2c_read_reg(RT_I2C_1, addr, cfg[i][0], &tmp_reg_value, 1);
		if(ret != RT_EOK)
		{
			debug_printf("read reg(0x%x) fail", cfg[i][0]);
			goto ERROR;
		}

		if(rt_memcmp((rt_uint8_t *)(cfg[i] + 1), &tmp_reg_value, 1))
		{
			debug_printf("check reg(0x%x) write result error", cfg[i][0]);
			goto ERROR;
		}
	}
	debug_printf("succeed to configure qmc5883l!!!");

	while(1)
	{
//        XH=read_reg(0x00);
//        YH=read_reg(0x02);
//        ZH=read_reg(0x04);
	    rt_hw_i2c_read_reg(RT_I2C_1, addr, 0x06, &tmp_reg_value, 1);
//	    debug_printf("reg 0x06=0x%x",tmp_reg_value);
	    rt_thread_delay(1);
	    if(tmp_reg_value & 0x02)
	    {
	        debug_printf("output data OVL!!!");
	    }
	    if(tmp_reg_value & 0x01)
	    {
//            debug_printf("X:0x%04x, Y:0x%04x, Z:0x%04x",
//                    read_reg(0x03), read_reg(0x05), read_reg(0x07));
//                      read_reg(0x00), read_reg(0x02), read_reg(0x04));
	        XH=read_reg(0x00)-6500;
	        YH=read_reg(0x02);
	        ZH=read_reg(0x04)-3300;
	        angle=(XH>0?atanf((float)ZH/(float)XH)+1.570795:4.712385+atanf((float)ZH/(float)XH));
//	        debug_printf("X:%04d, Y:%04d, Z:%04d",XH,YH,ZH);
	        angle_int=((int)(angle*100)+864)%628;
//            debug_printf("angle_magne=%d",angle_int);
            rt_mb_send(Angle_mb1, angle_int);
            rt_mb_send(Angle_mb2, angle_int);
	    }
		rt_thread_delay(10);
	}

ERROR:
	return;
}

void test1314_init(void)
{
    Angle_mb1 = rt_mb_create("Angle_mb1", 1,RT_IPC_FLAG_FIFO);
    if(!Angle_mb1)
    {
        debug_printf("Can't create Angle_mb1");
        return;
    }
    Angle_mb2 = rt_mb_create("Angle_mb2", 1,RT_IPC_FLAG_FIFO);
    if(!Angle_mb2)
    {
        debug_printf("Can't create Angle_mb2");
        return;
    }
	rt_thread_t tid;
	tid = rt_thread_create("1314_t",
			test1314_thread_entry, RT_NULL,
			RT_THREAD_STACK_SIZE_NORMAL, 26, RT_THREAD_TICK_NORMAL);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
}


