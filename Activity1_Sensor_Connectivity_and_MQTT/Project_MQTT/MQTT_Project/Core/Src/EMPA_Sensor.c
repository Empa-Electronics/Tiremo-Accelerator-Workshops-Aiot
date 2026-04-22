/*
 * EMPA_Sensor.c
 *
 *  Created on: May 19, 2024
 *      Author: bilalk
 */

#include "EMPA_Sensor.h"
#include "Empa_DataCollector.h"

uint8_t counter_timer = 0;
uint8_t flag_working = 0;
uint16_t counter_state = 0;
extern uint8_t Flag_DataCollector;
extern SensorMode sensor_mode;

//SHT4X Start
uint32_t sht4x_serialnumber;
int32_t temperature = 0;
int32_t humidity = 0;
//SHT4X End

//ISM330IS Start
ISM330IS_Object_t ism330_obj_o;
ISM330IS_IO_t ism330_ctx;
uint8_t ism330_id;
ISM330IS_Capabilities_t ism330_cap;

imu_data imu_sensor_data;
//ISM330IS End

//IMP34DT05 Start
#define REC_BUFF_SIZE        (2048U)
__IO uint32_t  DmaRecHalfBuffCplt  = 0;
__IO uint32_t  DmaRecBuffCplt      = 0;
MDF_DmaConfigTypeDef         DMAConfig;
int16_t        RecBuff[REC_BUFF_SIZE * 2U];
extern DMA_QListTypeDef MDFQueue;
extern MDF_FilterConfigTypeDef AdfFilterConfig0;
extern MDF_HandleTypeDef AdfHandle0;
extern DMA_HandleTypeDef handle_GPDMA1_Channel11;
//IMP34DT05 End

uint8_t EMPA_SensorInit(void) {
//SHT4X Init Start
	if (BSP_I2C1_Init() != HAL_OK)
		return 1;
	if (sht4x_serial_number(&sht4x_serialnumber) == 1)
		return 1;
//SHT4X Init End

//ISM330IS Init Start
	ism330_ctx.BusType = ISM330IS_I2C_BUS;
	ism330_ctx.Address = ISM330IS_I2C_ADD_H;
	ism330_ctx.Init = BSP_I2C1_Init;
	ism330_ctx.DeInit = BSP_I2C1_DeInit;
	ism330_ctx.ReadReg = BSP_I2C1_ReadReg;
	ism330_ctx.WriteReg = BSP_I2C1_WriteReg;
	ism330_ctx.GetTick = BSP_GetTick;

	if (ISM330IS_RegisterBusIO(&ism330_obj_o, &ism330_ctx) != ISM330IS_OK)
		return 1;

	if (ISM330IS_ReadID(&ism330_obj_o, &ism330_id) != ISM330IS_OK)
		return 1;

	if (ism330_id != ISM330IS_ID)
		return 1;

	if (ISM330IS_Init(&ism330_obj_o) != ISM330IS_OK)
		return 1;

	if (ISM330IS_ACC_Enable(&ism330_obj_o) != ISM330IS_OK)
		return 1;

	if (ISM330IS_GYRO_Enable(&ism330_obj_o) != ISM330IS_OK)
		return 1;

	HAL_Delay(300);
//ISM330IS Init End

//IMP34DT05 Init Start
	MX_MDFQueue_Config();
	HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel11, &MDFQueue);
	MDF_DMAConfig();

//IMP34DT05 Init End
	return 0;

}

uint8_t EMPA_SensorProcess(void) {
	// SHT40 Sensor Process Start
	if (sht40_sensor_process(&temperature, &humidity) != HAL_OK)
		return 1;
	// SHT40 Sensor Process End

	//ISM330IS Sensor Process Start
	if (ism330_sensor_process(&imu_sensor_data) != HAL_OK)
		return 1;
	//ISM330IS Sensor Process End

//	//IMP34DT05 Sensor Process Start
	if(imp34_sensor_process() != HAL_OK) return 1;
//	//IMP34DT05 Sensor Process End

	return 0;
}

void EMPA_SensorPrint(void) {
	//Print Start
	printf("\e[1;1H\e[2J");
	//Print End

	//SHT40 Print Start
	printf("========== SHT40 (Temperature & Humidity) ==========\n");
	printf("Temperature : %.2f C\n", temperature / 1000.0f);
	printf("Humidity    : %.2f %%\n\n", humidity / 1000.0f);
	//SHT40 Print End

	//ISM330IS Print Start
	printf("========== ISM330IS (Accelerometer) ==========\n");
	printf("ACC X: %ld\n", imu_sensor_data.acc.x);
	printf("ACC Y: %ld\n", imu_sensor_data.acc.y);
	printf("ACC Z: %ld\n\n", imu_sensor_data.acc.z);

	printf("========== ISM330IS (Gyroscope) ==========\n");
	printf("GYRO X: %ld\n", imu_sensor_data.gyro.x);
	printf("GYRO Y: %ld\n", imu_sensor_data.gyro.y);
	printf("GYRO Z: %ld\n\n", imu_sensor_data.gyro.z);
	//ISM330IS Print End

	//IMP34DT05 Print Start
	printf("========== IMP34DT05 (Microphone Raw Data) ==========\n");
	uint32_t bufferSize = REC_BUFF_SIZE;

    for(int i=0; i<bufferSize; i++)
    {
    	printf("%d ",RecBuff[i]);
    }
	//IMP34DT05 Print End
    HAL_Delay(500);
}

uint8_t sht40_sensor_process(int32_t *temp, int32_t *hum) {
	if (sht4x_measure_high_precision(temp, hum) == 0) {
		return 0;
	} else
		return 1;
}

uint8_t ism330_sensor_process(imu_data *sensor_data) {
	ISM330IS_Axes_t ism330_axes;
	if (ISM330IS_ACC_GetAxes(&ism330_obj_o, &ism330_axes) == 0) {
		sensor_data->acc.x = ism330_axes.x;
		sensor_data->acc.y = ism330_axes.y;
		sensor_data->acc.z = ism330_axes.z;
	} else
		return 1;

	if (ISM330IS_GYRO_GetAxes(&ism330_obj_o, &ism330_axes) == 0) {
		sensor_data->gyro.x = ism330_axes.x / 100;
		sensor_data->gyro.y = ism330_axes.y / 100;
		sensor_data->gyro.z = ism330_axes.z / 100;
	} else
		return 1;
	return 0;
}

uint8_t imp34_sensor_process(void) {

    if (HAL_MDF_AcqStart_DMA(&AdfHandle0, &AdfFilterConfig0, &DMAConfig) != HAL_OK)
    {
        Error_Handler();
    }
    while(DmaRecBuffCplt != 1);
    if (DmaRecBuffCplt == 1)
    {
        HAL_MDF_AcqStop_DMA(&AdfHandle0);
    }
    DmaRecBuffCplt = 0;

	return 0;
}

void MDF_DMAConfig(void)
{
  /* Initialize DMA configuration parameters */
  DMAConfig.Address                              = (uint32_t)&RecBuff[0];
  DMAConfig.DataLength                           = (REC_BUFF_SIZE * 4U);
  DMAConfig.MsbOnly                              = ENABLE;
}

void HAL_MDF_AcqHalfCpltCallback(MDF_HandleTypeDef *hmdf)
{
    DmaRecHalfBuffCplt = 1;
}

void HAL_MDF_AcqCpltCallback(MDF_HandleTypeDef *hmdf)
{
    DmaRecBuffCplt = 1;
}

