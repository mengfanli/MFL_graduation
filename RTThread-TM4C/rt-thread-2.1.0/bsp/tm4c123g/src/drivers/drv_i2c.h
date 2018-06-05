/*
 * drv_i2c.h
 *
 *  Created on: 2016Äê7ÔÂ24ÈÕ
 *      Author: dbw
 */

#ifndef SRC_DRIVERS_DRV_I2C_H_
#define SRC_DRIVERS_DRV_I2C_H_

//#define RT_I2C_0	0
#define RT_I2C_1	1

rt_err_t rt_hw_i2c_init(void);
rt_err_t rt_hw_i2c_write_reg(rt_uint32_t i2c_num, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len);
rt_err_t rt_hw_i2c_read_reg(rt_uint32_t i2c_num, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len);

void rt_hw_i2c_test(void);

#endif /* SRC_DRIVERS_DRV_I2C_H_ */
