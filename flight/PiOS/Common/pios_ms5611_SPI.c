

#define PIOS_MS5611_OVERSAMPLING oversampling

#include "pios.h"
#include "pios_ms5611.h"
/* Glocal Variables */
ConversionTypeTypeDef CurrentRead;

/* Local Variables */
MS5611CalibDataTypeDef CalibData;

/* Straight from the datasheet */

static uint32_t RawTemperature;
static uint32_t RawPressure;
static int64_t Pressure;
static int64_t Temperature;

//static int32_t PIOS_MS5611_Read(uint8_t address, uint8_t * buffer, uint8_t len);
//static int32_t PIOS_MS5611_WriteCommand(uint8_t command);

// Move into proper driver structure with cfg stored
static uint32_t oversampling;
static const struct pios_ms5611_cfg * dev_cfg;

//! Private functions
static struct MS5611_dev * PIOS_MS5611_alloc(void);


//static int32_t i2c_id;

static enum pios_ms5611_osr osr = MS5611_OSR_256;



enum pios_MS5611_dev_magic {
	PIOS_MS5611_DEV_MAGIC = 0x26092003,
};

struct MS5611_dev {
	uint32_t spi_id;
	uint32_t slave_num;
	const struct pios_MS5611_cfg * cfg;
	enum pios_MS5611_dev_magic magic;
};

//! Global structure for this device device
static struct MS5611_dev * dev;
/**
 * @brief Allocate a new device
 */
static struct MS5611_dev * PIOS_MS5611_alloc(void)
{
	struct MS5611_dev * MS5611_dev;

	MS5611_dev = (struct MS5611_dev *)pvPortMalloc(sizeof(*MS5611_dev));
	if (!MS5611_dev) return (NULL);

	MS5611_dev->magic = PIOS_MS5611_DEV_MAGIC;
/*
	MS5611_dev->queue = xQueueCreate(PIOS_MS5611_MAX_DOWNSAMPLE, sizeof(struct pios_MS5611_data));
	if(MS5611_dev->queue == NULL) {
		vPortFree(MS5611_dev);
		return NULL;
	}
*/
	return(MS5611_dev);
}

/**
 * @brief Validate the handle to the spi device
 * @returns 0 for valid device or -1 otherwise
 */
static int32_t PIOS_MS5611_Validate(struct MS5611_dev * dev)
{
	if (dev == NULL)
		return -1;
	if (dev->magic != PIOS_MS5611_DEV_MAGIC)
		return -2;
	if (dev->spi_id == 0)
		return -3;
	return 0;
}

/**
 * @brief Claim the SPI bus for the accel communications and select this chip
 * @return 0 if successful, -1 for invalid device, -2 if unable to claim bus
 */
int32_t PIOS_MS5611_ClaimBus()
{
	if(PIOS_MS5611_Validate(dev) != 0)
		return -1;

	if(PIOS_SPI_ClaimBus(dev->spi_id) != 0)
		return -2;

	PIOS_SPI_RC_PinSet(dev->spi_id,dev->slave_num,0);
	return 0;
}

/**
 * @brief Release the SPI bus for the accel communications and end the transaction
 * @return 0 if successful
 */
int32_t PIOS_MS5611_ReleaseBus()
{
	if(PIOS_MS5611_Validate(dev) != 0)
		return -1;

	PIOS_SPI_RC_PinSet(dev->spi_id,dev->slave_num,1);

	return PIOS_SPI_ReleaseBus(dev->spi_id);
}


/**
 * Initialise the MS5611 sensor
 */
int32_t ms5611_read_flag;
int32_t PIOS_MS5611_Init(uint32_t spi_id, uint32_t slave_num, const struct pios_ms5611_cfg * cfg)
{
	oversampling = cfg->oversampling;
	dev_cfg = cfg;	// Store cfg before enabling interrupt

	dev = PIOS_MS5611_alloc();
	if(dev == NULL)
		return -1;

	dev->spi_id = spi_id;
	dev->slave_num = slave_num;

	/* Configure the MS5611 Sensor */
	PIOS_SPI_SetClockSpeed(dev->spi_id, PIOS_SPI_PRESCALER_256);

	// RESET ms5611
	uint8_t out[] = {MS5611_RESET};  //0x1e

	if(PIOS_MS5611_ClaimBus() < 0)
		return -2;

	if(PIOS_SPI_TransferBlock(dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
		PIOS_MS5611_ReleaseBus();
			return -3;
		}
	PIOS_MS5611_ReleaseBus();

	PIOS_DELAY_WaitmS(4);
	// RESET ms5611

// Read Calibration parameters
//	uint8_t out1[] = {MS5611_CALIB_ADDR, 0, 0};  //0xA2 - First sample is factory stuff
/*
	if(PIOS_MS5611_ClaimBus() < 0)
		return -2;

	if(PIOS_SPI_TransferBlock(dev->spi_id,out1,NULL,sizeof(out1),NULL) < 0) {
		PIOS_MS5611_ReleaseBus();
			return -3;
		}
*/
	uint8_t data[2];



//	printf(" Calibration param. ");
	/* Calibration parameters */
	for (int i = 0; i < 6; i++) {
		if(PIOS_MS5611_ClaimBus() < 0)
			return -2;

		if(PIOS_SPI_TransferByte(dev->spi_id, MS5611_CALIB_ADDR + i*2) < 0) {
				PIOS_MS5611_ReleaseBus();
					return -3;
		}

		if(PIOS_SPI_TransferBlock(dev->spi_id,NULL,data,sizeof(data),NULL) < 0) {
				PIOS_MS5611_ReleaseBus();
					return -3;
				}
		CalibData.C[i] = (data[0] << 8) | data[1];
//		printf(" %02x%02x ",(uint8_t )(data[0]),(uint8_t )(data[1]));
		PIOS_MS5611_ReleaseBus();
	}


	return 0;
}

/**
* Start the ADC conversion
* \param[in] PresOrTemp BMP085_PRES_ADDR or BMP085_TEMP_ADDR
* \return 0 for success, -1 for failure (conversion completed and not read)
*/
int32_t PIOS_MS5611_StartADC(ConversionTypeTypeDef Type)
{
	uint8_t outP[] = {MS5611_PRES_ADDR + osr};  //0x40
	uint8_t outT[] = {MS5611_TEMP_ADDR + osr};  //0x50

	if(PIOS_MS5611_ClaimBus() < 0)
		return -2;

	/* Start the conversion */
	if (Type == TemperatureConv) {
		if(PIOS_SPI_TransferBlock(dev->spi_id,outT,NULL,sizeof(outT),NULL) < 0) {
			PIOS_MS5611_ReleaseBus();
				return -3;
			}
//		while (PIOS_MS5611_WriteCommand(MS5611_TEMP_ADDR + osr) != 0)
//			continue;
	} else if (Type == PressureConv) {
		if(PIOS_SPI_TransferBlock(dev->spi_id,outP,NULL,sizeof(outP),NULL) < 0) {
			PIOS_MS5611_ReleaseBus();
				return -3;
			}
//		while (PIOS_MS5611_WriteCommand(MS5611_PRES_ADDR + osr) != 0)
//			continue;
	}

	CurrentRead = Type;
	PIOS_MS5611_ReleaseBus();
	return 0;
}

/**
 * @brief Return the delay for the current osr
 */
int32_t PIOS_MS5611_GetDelay() {
	switch(osr) {
		case MS5611_OSR_256:
			return 2;
		case MS5611_OSR_512:
			return 2;
		case MS5611_OSR_1024:
			return 3;
		case MS5611_OSR_2048:
			return 5;
		case MS5611_OSR_4096:
			return 10;
		default:
			break;
	}
	return 10;
}
/**
* Read the ADC conversion value (once ADC conversion has completed)
* \param[in] PresOrTemp BMP085_PRES_ADDR or BMP085_TEMP_ADDR
* \return 0 if successfully read the ADC, -1 if failed
*/
int32_t PIOS_MS5611_ReadADC(void)
{
//	uint8_t Data[3];
/*
	Data[0] = 0;
	Data[1] = 0;
	Data[2] = 0;
*/
	uint8_t out[] = {MS5611_ADC_READ};  //0x00
	uint8_t Data[] = {0x00, 0x00, 0x00};

	static int64_t deltaTemp;

	if(PIOS_MS5611_ClaimBus() < 0)
		return -2;
	/* Read and store the 16bit result */
	if (CurrentRead == TemperatureConv) {
		/* Read the temperature conversion */
		if(PIOS_SPI_TransferBlock(dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
			PIOS_MS5611_ReleaseBus();
				return -1;
			}
		if(PIOS_SPI_TransferBlock(dev->spi_id,NULL,Data,sizeof(Data),NULL) < 0) {
			PIOS_MS5611_ReleaseBus();
				return -1;
			}

		PIOS_MS5611_ReleaseBus();
//		if (PIOS_MS5611_Read(MS5611_ADC_READ, Data, 3) != 0)
//			return -1;

		RawTemperature = (Data[0] << 16) | (Data[1] << 8) | Data[2];

		deltaTemp = RawTemperature - (CalibData.C[4] << 8);
		Temperature = 2000l + ((deltaTemp * CalibData.C[5]) >> 23);

	} else {
		int64_t Offset;
		int64_t Sens;

		if(PIOS_SPI_TransferBlock(dev->spi_id,out,NULL,sizeof(out),NULL) < 0) {
			PIOS_MS5611_ReleaseBus();
				return -1;
			}
		if(PIOS_SPI_TransferBlock(dev->spi_id,NULL,Data,sizeof(Data),NULL) < 0) {
			PIOS_MS5611_ReleaseBus();
				return -1;
			}

		PIOS_MS5611_ReleaseBus();
		/* Read the pressure conversion */
//		if (PIOS_MS5611_Read(MS5611_ADC_READ, Data, 3) != 0)
//			return -1;
		RawPressure = ((Data[0] << 16) | (Data[1] << 8) | Data[2]);

		Offset = (((int64_t) CalibData.C[1]) << 16) + ((((int64_t) CalibData.C[3]) * deltaTemp) >> 7);
		Sens = ((int64_t) CalibData.C[0]) << 15;
		Sens = Sens + ((((int64_t) CalibData.C[2]) * deltaTemp) >> 8);

		Pressure = (((((int64_t) RawPressure) * Sens) >> 21) - Offset) >> 15;
	}
	return 0;
}

/**
 * Return the most recently computed temperature in kPa
 */
float PIOS_MS5611_GetTemperature(void)
{
	return ((float) Temperature) / 100.0f;
}

/**
 * Return the most recently computed pressure in kPa
 */
float PIOS_MS5611_GetPressure(void)
{
	return ((float) Pressure) / 1000.0f;
}


