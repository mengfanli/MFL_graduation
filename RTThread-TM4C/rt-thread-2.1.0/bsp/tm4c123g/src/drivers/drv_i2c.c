/*
 * drv_i2c.c
 *
 *  Created on: 2016Äê7ÔÂ24ÈÕ
 *      Author: dbw
 */

/*
 * pin assignment
 *
 *-------------------------
 *  GPIO    |    I2C
 *-------------------------
 *  PA6     |   I2C1SCL
 *  PA7     |   I2C1SDA
 *-------------------------
 *
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "debug.h"
#include "board.h"

#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/i2c.h"

#include "drv_i2c.h"

#define I2C1_GPIO_PERIPH            SYSCTL_PERIPH_GPIOA
#define I2C1_GPIO_PORT              GPIO_PORTA_BASE
#define I2C1_GPIO_PIN_CFG_SCL       GPIO_PA6_I2C1SCL
#define I2C1_GPIO_PIN_CFG_SDA       GPIO_PA7_I2C1SDA
#define I2C1_GPIO_PIN_NUM_SCL       GPIO_PIN_6
#define I2C1_GPIO_PIN_NUM_SDA       GPIO_PIN_7

rt_err_t i2c_wait_and_check_err(rt_uint32_t i2c_base);
rt_uint32_t i2c_get_base(rt_uint32_t i2c_num);
rt_err_t i2c_write_reg(rt_uint32_t i2c_base, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len);
rt_err_t i2c_read_reg(rt_uint32_t i2c_base, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len);

rt_err_t rt_hw_i2c_init(void)
{

    //
    // Enable the I2C1 peripheral
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    //
    // Wait for the I2C0 module to be ready.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C1));

    //
    // Configure gpio
    //
    SysCtlPeripheralEnable(I2C1_GPIO_PERIPH);
    GPIOPinConfigure(I2C1_GPIO_PIN_CFG_SCL);
    GPIOPinConfigure(I2C1_GPIO_PIN_CFG_SDA);
    GPIOPinTypeI2C(I2C1_GPIO_PORT, I2C1_GPIO_PIN_NUM_SDA);
    GPIOPinTypeI2CSCL(I2C1_GPIO_PORT, I2C1_GPIO_PIN_NUM_SCL);

    //
    // Initialize Master and Slave
    //
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);
    //
    // Enable i2c
    //
    I2CMasterEnable(I2C1_BASE);

    return RT_EOK;
}

void rt_hw_i2c_test(void)
{
    rt_uint8_t slave_addr = 0x68;
    rt_uint8_t reg_addr = 0x75;
    rt_uint8_t data;
    rt_err_t ret;

    ret = i2c_read_reg(I2C1_BASE, slave_addr, reg_addr, &data, 1);

    if(ret == RT_EOK)
    {
    	debug_printf("who am i : 0x%x", data);
    }
    else
    {
    	debug_printf("i2c test error");
    }
}

rt_uint32_t i2c_get_base(rt_uint32_t i2c_num)
{
	if(i2c_num == RT_I2C_1)
	{
		return I2C1_BASE;
	}
	else
	{
		return 0;
	}
}

rt_err_t rt_hw_i2c_write_reg(rt_uint32_t i2c_num, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len)
{
	rt_uint32_t i2c_base;

	i2c_base = i2c_get_base(i2c_num);

	if(i2c_base == 0)
	{
		return -RT_ENOSYS;
	}

	return i2c_write_reg(i2c_base, slave_addr, reg_addr, data, len);
}

rt_err_t i2c_write_reg(rt_uint32_t i2c_base, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len)
{
    //
    // Specify slave address
    //
    I2CMasterSlaveAddrSet(i2c_base, slave_addr, false);
    //
    // Place the character to be sent in the data register
    //
    I2CMasterDataPut(i2c_base, reg_addr);
    //
    // Initiate send of character from Master to Slave
    //
    I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_START);
    //
    // Delay until transmission completes
    //
    if(i2c_wait_and_check_err(i2c_base) != RT_EOK)
    {
    	goto ERROR;
    }

    for(rt_size_t i = 0; i < len; i++)
    {
    	I2CMasterDataPut(i2c_base, data[i]);
        if(i < len - 1)
        {
            I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_CONT);
        }
        else
        {
            I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_FINISH);
        }
        if(i2c_wait_and_check_err(i2c_base) != RT_EOK)
        {
        	goto ERROR;
        }
    }

    return RT_EOK;

ERROR:
	I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
	while(I2CMasterBusy(i2c_base));
	return -RT_EIO;
}

rt_err_t rt_hw_i2c_read_reg(rt_uint32_t i2c_num, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len)
{
	rt_uint32_t i2c_base;

	i2c_base = i2c_get_base(i2c_num);

	if(i2c_base == 0)
	{
		return -RT_ENOSYS;
	}

	return i2c_read_reg(i2c_base, slave_addr, reg_addr, data, len);
}

rt_err_t i2c_read_reg(rt_uint32_t i2c_base, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len)
{
    //
    // Specify slave address
    //
    I2CMasterSlaveAddrSet(i2c_base, slave_addr, false);
    //
    // Place the character to be sent in the data register
    //
    I2CMasterDataPut(i2c_base, reg_addr);
    //
    // Initiate send of character from Master to Slave
    //
    I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_START);
    //
    // Delay until transmission completes
    //
    if(i2c_wait_and_check_err(i2c_base) != RT_EOK)
    {
    	I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
		while(I2CMasterBusy(i2c_base));
		return -RT_EIO;
    }

    I2CMasterSlaveAddrSet(i2c_base, slave_addr, true);
    for(rt_size_t i = 0; i < len; i++)
    {
        if(i == 0)
        {
            I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_START);
        }
        else if(i < len - 1)
        {
            I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        }
        else
        {
        	I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        }
        if(i2c_wait_and_check_err(i2c_base) != RT_EOK)
        {
        	goto RECV_ERROR;
        }

        data[i] = I2CMasterDataGet(i2c_base);
    }

    if(len == 1)
    {
    	I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    }

    return RT_EOK;

RECV_ERROR:
	I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
	while(I2CMasterBusy(i2c_base));
	return -RT_EIO;
}

rt_err_t i2c_wait_and_check_err(rt_uint32_t i2c_base)
{
	while(I2CMasterBusy(i2c_base));
    uint32_t err = I2CMasterErr(i2c_base);
    if(err)
    {
        debug_printf("I2C master error:0x%x", err);
//        RT_ASSERT(!err);

        return -RT_EIO;
    }
    else
    {
    	return RT_EOK;
    }
}
