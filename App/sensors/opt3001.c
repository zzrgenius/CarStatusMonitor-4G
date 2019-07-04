/**
 * opt3001.c - Texas Instruments OPT3001 Light Sensor
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Andreas Dannenberg <dannenberg@ti.com>
 * Based on previous work from: Felipe Balbi <balbi@ti.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 of the License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <stm32f4xx_hal.h>
 #include <stdbool.h>
 #include "mpu9250_driver.h" 
#define OPT3001_RESULT		0x00
#define OPT3001_CONFIGURATION	0x01
#define OPT3001_LOW_LIMIT	0x02
#define OPT3001_HIGH_LIMIT	0x03
#define OPT3001_MANUFACTURER_ID	0x7e
#define OPT3001_DEVICE_ID	0x7f

#define OPT3001_CONFIGURATION_RN_MASK	(0xf << 12)
#define OPT3001_CONFIGURATION_RN_AUTO	(0xc << 12)

#define OPT3001_CONFIGURATION_CT	BIT(11)

#define OPT3001_CONFIGURATION_M_MASK	(3 << 9)
#define OPT3001_CONFIGURATION_M_SHUTDOWN (0 << 9)
#define OPT3001_CONFIGURATION_M_SINGLE	(1 << 9)
#define OPT3001_CONFIGURATION_M_CONTINUOUS (2 << 9) /* also 3 << 9 */

#define OPT3001_CONFIGURATION_OVF	BIT(8)
#define OPT3001_CONFIGURATION_CRF	BIT(7)
#define OPT3001_CONFIGURATION_FH	BIT(6)
#define OPT3001_CONFIGURATION_FL	BIT(5)
#define OPT3001_CONFIGURATION_L		BIT(4)
#define OPT3001_CONFIGURATION_POL	BIT(3)
#define OPT3001_CONFIGURATION_ME	BIT(2)

#define OPT3001_CONFIGURATION_FC_MASK	(3 << 0)

/* The end-of-conversion enable is located in the low-limit register */
#define OPT3001_LOW_LIMIT_EOC_ENABLE	0xc000

#define OPT3001_REG_EXPONENT(n)		((n) >> 12)
#define OPT3001_REG_MANTISSA(n)		((n) & 0xfff)

#define OPT3001_INT_TIME_LONG		800000
#define OPT3001_INT_TIME_SHORT		100000
#define opt_addr  0x45
/*
 * Time to wait for conversion result to be ready. The device datasheet
 * sect. 6.5 states results are ready after total integration time plus 3ms.
 * This results in worst-case max values of 113ms or 883ms, respectively.
 * Add some slack to be on the safe side.
 */
#define OPT3001_RESULT_READY_SHORT	150
#define OPT3001_RESULT_READY_LONG	1000

struct opt3001 {
	struct i2c_client	*client;
	struct device		*dev;

 
	bool			ok_to_ignore_lock;
	bool			result_ready;
 
	uint16_t			result;

	uint32_t			int_time;
	uint32_t			mode;

	uint16_t			high_thresh_mantissa;
	uint16_t			low_thresh_mantissa;

	uint8_t			high_thresh_exp;
	uint8_t			low_thresh_exp;

	bool			use_irq;
};

struct opt3001_scale {
	int	val;
	int	val2;
};

static const struct opt3001_scale opt3001_scales[] = {
	{
		.val = 40,
		.val2 = 950000,
	},
	{
		.val = 81,
		.val2 = 900000,
	},
	{
		.val = 163,
		.val2 = 800000,
	},
	{
		.val = 327,
		.val2 = 600000,
	},
	{
		.val = 655,
		.val2 = 200000,
	},
	{
		.val = 1310,
		.val2 = 400000,
	},
	{
		.val = 2620,
		.val2 = 800000,
	},
	{
		.val = 5241,
		.val2 = 600000,
	},
	{
		.val = 10483,
		.val2 = 200000,
	},
	{
		.val = 20966,
		.val2 = 400000,
	},
	{
		.val = 83865,
		.val2 = 600000,
	},
};

static int opt3001_find_scale(const struct opt3001 *opt, int val,
		int val2, uint8_t *exponent)
{
	int i;

	for (i = 0; i < sizeof(opt3001_scales)/2; i++) {
		const struct opt3001_scale *scale = &opt3001_scales[i];

		/*
		 * Combine the integer and micro parts for comparison
		 * purposes. Use milli lux precision to avoid 32-bit integer
		 * overflows.
		 */
		if ((val * 1000 + val2 / 1000) <=
				(scale->val * 1000 + scale->val2 / 1000)) {
			*exponent = i;
			return 0;
		}
	}

	return -1;
}

static void opt3001_to_iio_ret(struct opt3001 *opt, uint8_t exponent,
		uint16_t mantissa, int *val, int *val2)
{
	int lux;

	lux = 10 * (mantissa << exponent);
	*val = lux / 1000;
	*val2 = (lux - (*val * 1000)) * 1000;
}

static void opt3001_set_mode(struct opt3001 *opt, uint16_t *reg, uint16_t mode)
{
	*reg &= ~OPT3001_CONFIGURATION_M_MASK;
	*reg |= mode;
	opt->mode = mode;
}
 
 
//	return  Sensors_I2C_WriteRegister(i2c_addr<<1, reg_addr,length,reg_data );
//    return Sensors_I2C_ReadRegister(i2c_addr<<1, reg_addr, length,reg_data) ;
uint8_t i2c_smbus_read_word_swapped(uint8_t i2c_addr, uint16_t cmd)
{
	uint8_t reg_data;
	Sensors_I2C_ReadRegister(i2c_addr<<1,cmd,1,&reg_data);
	return reg_data;
	
}
 int i2c_smbus_write_word_swapped(uint8_t i2c_addr, uint16_t reg_addr,uint16_t reg_data)
 {
	  return  Sensors_I2C_WriteRegister(i2c_addr<<1, reg_addr,2,(unsigned char*)&reg_data );

 }

static int opt3001_get_lux(struct opt3001 *opt, int *val, int *val2)
{
	int ret;
	uint16_t mantissa;
	uint16_t reg;
	uint8_t exponent;
	uint16_t value;
	long timeout;

	if (opt->use_irq) 
	{
		/*
		 * Enable the end-of-conversion interrupt mechanism. Note that
		 * doing so will overwrite the low-level limit value however we
		 * will restore this value later on.
		 */
		ret = i2c_smbus_write_word_swapped(opt_addr,OPT3001_LOW_LIMIT,OPT3001_LOW_LIMIT_EOC_ENABLE);
		if (ret < 0) {
			printf(  "failed to write register %02x\n",					OPT3001_LOW_LIMIT);
			return ret;
		}

		/* Allow IRQ to access the device despite lock being set */
		 
	}

	/* Reset data-ready indicator flag */
 
	/* Configure for single-conversion mode and start a new conversion */
	ret = i2c_smbus_read_word_swapped(opt_addr, OPT3001_CONFIGURATION);
	if (ret < 0) {
		printf( "failed to read register %02x\n",
				OPT3001_CONFIGURATION);
		goto err;
	}

	reg = ret;
	opt3001_set_mode(opt, &reg, OPT3001_CONFIGURATION_M_SINGLE);

	ret = i2c_smbus_write_word_swapped(opt_addr, OPT3001_CONFIGURATION,
			reg);
	if (ret < 0) {
		printf("failed to write register %02x\n",
				OPT3001_CONFIGURATION);
		goto err;
	}

	if (opt->use_irq) {
		/* Wait for the IRQ to indicate the conversion is complete */
		 
	} else {
		/* Sleep for result ready time */
		timeout = (opt->int_time == OPT3001_INT_TIME_SHORT) ?
			OPT3001_RESULT_READY_SHORT : OPT3001_RESULT_READY_LONG;
		HAL_Delay(timeout);

		/* Check result ready flag */
		ret = i2c_smbus_read_word_swapped(opt_addr,
						  OPT3001_CONFIGURATION);
		if (ret < 0) {
			printf("failed to read register %02x\n",
				OPT3001_CONFIGURATION);
			goto err;
		}

		if (!(ret & OPT3001_CONFIGURATION_CRF)) {
			ret = -2;
			goto err;
		}

		/* Obtain value */
		ret = i2c_smbus_read_word_swapped(opt_addr, OPT3001_RESULT);
		if (ret < 0) {
			printf("failed to read register %02x\n",
				OPT3001_RESULT);
			goto err;
		}
		opt->result = ret;
		opt->result_ready = true;
	}

err:
	if (opt->use_irq)
		/* Disallow IRQ to access the device while lock is active */
		opt->ok_to_ignore_lock = false;

	if (ret == 0)
		return -2;
	else if (ret < 0)
		return ret;

	if (opt->use_irq) {
		/*
		 * Disable the end-of-conversion interrupt mechanism by
		 * restoring the low-level limit value (clearing
		 * OPT3001_LOW_LIMIT_EOC_ENABLE). Note that selectively clearing
		 * those enable bits would affect the actual limit value due to
		 * bit-overlap and therefore can't be done.
		 */
		value = (opt->low_thresh_exp << 12) | opt->low_thresh_mantissa;
		ret = i2c_smbus_write_word_swapped(opt_addr,
						   OPT3001_LOW_LIMIT,
						   value);
		if (ret < 0) {
			printf("failed to write register %02x\n",
					OPT3001_LOW_LIMIT);
			return ret;
		}
	}

	exponent = OPT3001_REG_EXPONENT(opt->result);
	mantissa = OPT3001_REG_MANTISSA(opt->result);

 
	return 0;
}

 
 

 
static int opt3001_read_id(struct opt3001 *opt)
{
	char manufacturer[2];
	uint16_t device_id;
	int ret;

	ret = i2c_smbus_read_word_swapped(opt_addr, OPT3001_MANUFACTURER_ID);
	if (ret < 0) {
		printf("failed to read register %02x\n",
				OPT3001_MANUFACTURER_ID);
		return ret;
	}

	manufacturer[0] = ret >> 8;
	manufacturer[1] = ret & 0xff;

	ret = i2c_smbus_read_word_swapped(opt_addr, OPT3001_DEVICE_ID);
	if (ret < 0) {
		printf("failed to read register %02x\n",
				OPT3001_DEVICE_ID);
		return ret;
	}

	device_id = ret;

	printf("Found %c%c OPT%04x\n", manufacturer[0],
			manufacturer[1], device_id);

	return 0;
}

static int opt3001_configure(struct opt3001 *opt)
{
	int ret;
	uint16_t reg;

	ret = i2c_smbus_read_word_swapped(opt_addr, OPT3001_CONFIGURATION);
	if (ret < 0) {
		printf( "failed to read register %02x\n",	OPT3001_CONFIGURATION);
		return ret;
	}

	reg = ret;

	/* Enable automatic full-scale setting mode */
	reg &= ~OPT3001_CONFIGURATION_RN_MASK;
	reg |= OPT3001_CONFIGURATION_RN_AUTO;

	/* Reflect status of the device's integration time setting */
	if (reg & OPT3001_CONFIGURATION_CT)
		opt->int_time = OPT3001_INT_TIME_LONG;
	else
		opt->int_time = OPT3001_INT_TIME_SHORT;

	/* Ensure device is in shutdown initially */
	opt3001_set_mode(opt, &reg, OPT3001_CONFIGURATION_M_SHUTDOWN);

	/* Configure for latched window-style comparison operation */
	reg |= OPT3001_CONFIGURATION_L;
	reg &= ~OPT3001_CONFIGURATION_POL;
	reg &= ~OPT3001_CONFIGURATION_ME;
	reg &= ~OPT3001_CONFIGURATION_FC_MASK;

	ret = i2c_smbus_write_word_swapped(opt_addr, OPT3001_CONFIGURATION,	reg);
	if (ret < 0) {
		printf( "failed to write register %02x\n",	OPT3001_CONFIGURATION);
		return ret;
	}

	ret = i2c_smbus_read_word_swapped(opt_addr, OPT3001_LOW_LIMIT);
	if (ret < 0) {
		printf( "failed to read register %02x\n",		OPT3001_LOW_LIMIT);
		return ret;
	}

	opt->low_thresh_mantissa = OPT3001_REG_MANTISSA(ret);
	opt->low_thresh_exp = OPT3001_REG_EXPONENT(ret);

	ret = i2c_smbus_read_word_swapped(opt_addr, OPT3001_HIGH_LIMIT);
	if (ret < 0) {
		printf( "failed to read register %02x\n",
				OPT3001_HIGH_LIMIT);
		return ret;
	}

	opt->high_thresh_mantissa = OPT3001_REG_MANTISSA(ret);
	opt->high_thresh_exp = OPT3001_REG_EXPONENT(ret);

	return 0;
}

static int opt3001_irq(int irq, void *_iio)
{
 
	int ret;

	 
	ret = i2c_smbus_read_word_swapped(opt_addr , OPT3001_CONFIGURATION);
	if (ret < 0) {
		printf("failed to read register %02x\n",OPT3001_CONFIGURATION);
		goto out;
	}

	if ((ret & OPT3001_CONFIGURATION_M_MASK) == OPT3001_CONFIGURATION_M_CONTINUOUS) 
	{
		 
	}

out:
	 
	return -1;
}

 

 
 
 
 