#ifndef __BMP280_I2C_H__
#define __BMP280_I2C_H__

#define DEFAULT_CHIP_ID    0X58
#define RESET_VALUE        0XB6

#define ID_ADDRESS			0XD0
#define RESET_ADDRESS		0XE0
#define STATUS_ADDRESS		0XF3
#define CTRL_MEAS_ADDRESS	0XF4
#define CONFIG_ADDRESS		0XF5
#define PRESSURE_MSB		0xF7
#define PRESSURE_LSB		0xF8
#define PRESSURE_XLSB		0xF9
#define TEMP_MSB			0xFA
#define TEMP_LSB			0xFB
#define TEMP_XLSB			0xFC

#define MODE_REG			0X00
#define OSRS_P_REG			0X02
#define OSRS_T_REG			0X05
#define FILTER_REG			0X02
#define T_SB_REG			0X05

#define CALIB_LO_REG		0x88
#define CALIB_HI_REG		0x9F

typedef enum {
	STANDBY_0M5,
    STANDBY_62M5,
    STANDBY_125M,
    STANDBY_250M,
    STANDBY_500M,
    STANDBY_1000M,
	STANDBY_2000M,
	STANDBY_4000M,
} bmp280_tsby_t;

typedef enum {
	SLEEP_MODE = 0,
	FORCED_MODE = 1,
	NORMAL_MODE = 3,
} bmp280_mode_t;

typedef enum {
	OVERSAMPLING_OFF,
	OVERSAMPLING_X1,
	OVERSAMPLING_X2,
	OVERSAMPLING_X4,
	OVERSAMPLING_X8,
	OVERSAMPLING_X16,
} bmp280_osrs_t;

typedef enum {
	IIR_NONE,
	IIR_X1,
	IIR_X2,
	IIR_X4,
	IIR_X8,
	IIR_X16,
} bmp280_iirf_t;

typedef struct {
	uint16_t T1;
	int16_t T2;
	int16_t T3;
	uint16_t P1;
	int16_t P2;
	int16_t P3;
	int16_t P4;
	int16_t P5;
	int16_t P6;
	int16_t P7;
	int16_t P8;
	int16_t P9;
} bmp280_calib_t;

typedef struct {
	bmp280_tsby_t t_sdby;
	bmp280_iirf_t filter;
} bmp280_config_t;

typedef struct {
	bmp280_osrs_t t_oversampling;
	bmp280_osrs_t p_oversampling;
	bmp280_mode_t mode;
} bmp280_ctrl_meas_t;

void i2c_master_init(uint8_t SCL_IO_PIN, uint8_t SDA_IO_PIN);

void bmp280_i2c_init(bmp280_config_t *cfg, bmp280_ctrl_meas_t *ctrl);

void bmp280_get_temp(float *temp);

void bmp280_get_pressure(float *pressure);

#endif
