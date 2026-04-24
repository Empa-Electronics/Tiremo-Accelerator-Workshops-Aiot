/**
 *******************************************************************************
 * @file        prv_user_code.c
 * @author      ABOV R&D Division
 * @brief       Dummy User Application Main
 *
 * Copyright 2024 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "DebugLibrary/debug_framework.h"
#include "hal_pcu.h"
#include "hal_i2c.h"
#include "hal_timer1.h"
#include "ESP32_AT_Test/ESP32_AT_Test.h"
#include "MQTT_Library/EMPA_MqttAws.h"
#include "Sensor/sensor.h"

extern void MqttPort_ABOV_Init(void);
extern void MqttPort_ABOV_TickIncrement(void);
extern void MP23ABS1_TimerHandler(void);

#define TC_SYSTICK_1MS      1000

/* Test LEDs Configuration */
#define LED_PORT_ID         PCU_ID_B
#define LED_DEBUG           PCU_PIN_ID_4   // PB4  - Debug Framework Status
#define LED_SHT40           PCU_PIN_ID_5   // PB5  - SHT40 Sensor Status
#define LED_BATTERY         PCU_PIN_ID_9   // PB9  - Battery Reading Status
#define LED_LIS2DE12        PCU_PIN_ID_10  // PB10 - Accelerometer Status
#define LED_MIC             PCU_PIN_ID_11  // PB11 - Microphone Status  
#define LED_ESP32           PCU_PIN_ID_12  // PB12 - ESP32 Module Status
#define LED_TEST1           PCU_PIN_ID_13  // PB13 - General Test LED 1
#define LED_TEST2           PCU_PIN_ID_14  // PB14 - General Test LED 2
#define LED_TEST3           PCU_PIN_ID_15  // PB15 - General Test LED 3

/* Include HAL header files for your target modules */
#define I2C_SCL_PORT        PCU_ID_B    // PB6
#define I2C_SCL_PIN         PCU_PIN_ID_6
#define I2C_SDA_PORT        PCU_ID_B    // PB7
#define I2C_SDA_PIN         PCU_PIN_ID_7
#define I2C_ALT_FUNCTION    PCU_ALT_1

/* UART_ID_2 Configuration (ESP32-AT Module) */
#define UART2_PORT          PCU_ID_A    // PA8=RXD, PA9=TXD
#define UART2_RXD_PIN       PCU_PIN_ID_8
#define UART2_TXD_PIN       PCU_PIN_ID_9
#define UART2_ALT_FUNCTION  PCU_ALT_1

extern uint32_t SystemCoreClock;
static volatile uint32_t s_un32SysTimerVal=0;

/**********************************************************************
 * @brief		ARM System Timer Interrupt Handler.
 * @param[in]	None
 * @return	    None
 * @note        Also calls MP23ABS1_TimerHandler for microphone sampling
 **********************************************************************/
void SysTick_Handler (void)
{
    static uint32_t s_un32TickCounter = 0;
    
    if (s_un32SysTimerVal)
    {
        s_un32SysTimerVal--;
    }
    
    /* MQTT port tick (1ms) - needed for UART receive timeout */
    MqttPort_ABOV_TickIncrement();
    
    /* mqtt_timer increment (when enabled) */
    if (mqtt_timer_en) {
        mqtt_timer++;
    }
    
    /* Call MP23ABS1 timer handler every 16 ticks (for 1kHz SysTick, this gives ~62.5us per sample) */
    s_un32TickCounter++;
    if (s_un32TickCounter >= 16)
    {
        s_un32TickCounter = 0;
        MP23ABS1_TimerHandler();
    }
}

/**********************************************************************
 * @brief		Waiting by time(ms)
 * @param[in]	un32TimeMS : Milisecond time to wait.
 * @return	    None
 **********************************************************************/
void SYSTICK_Wait (uint32_t un32TimeMS)
{
    s_un32SysTimerVal = un32TimeMS;
    while(s_un32SysTimerVal);
}

/**********************************************************************
 * @brief       Turn ON specific LED
 * @param[in]   LED_PIN : LED pin ID
 * @return      None
 **********************************************************************/
void LED_ON(PCU_PIN_ID_e LED_PIN)
{
    HAL_PCU_SetOutputBit(LED_PORT_ID, LED_PIN, PCU_OUTPUT_BIT_SET);
}

/**********************************************************************
 * @brief       Turn OFF specific LED
 * @param[in]   LED_PIN : LED pin ID
 * @return      None
 **********************************************************************/
void LED_OFF(PCU_PIN_ID_e LED_PIN)
{
    HAL_PCU_SetOutputBit(LED_PORT_ID, LED_PIN, PCU_OUTPUT_BIT_CLEAR);
}

/**********************************************************************
 * @brief       Blink specific LED
 * @param[in]   LED_PIN : LED pin ID
 * @param[in]   times : Number of blinks
 * @return      None
 **********************************************************************/
void LED_Blink(PCU_PIN_ID_e LED_PIN, uint32_t times)
{
    for(uint32_t i = 0; i < times; i++)
    {
        LED_ON(LED_PIN);
        SYSTICK_Wait(100);
        LED_OFF(LED_PIN);
        SYSTICK_Wait(100);
    }
}

/**********************************************************************
 * @brief       Configure GPIO Alternate Functions
 * @param[in]   None
 * @return      None
 **********************************************************************/
void GPIO_Config_Alt()
{
    /* Configure all Test LEDs as GPIO output (ALT_0) */
    HAL_PCU_SetAltMode(LED_PORT_ID, LED_DEBUG, PCU_ALT_0);
    HAL_PCU_SetAltMode(LED_PORT_ID, LED_SHT40, PCU_ALT_0);
    HAL_PCU_SetAltMode(LED_PORT_ID, LED_BATTERY, PCU_ALT_0);
    HAL_PCU_SetAltMode(LED_PORT_ID, LED_LIS2DE12, PCU_ALT_0);
    HAL_PCU_SetAltMode(LED_PORT_ID, LED_MIC, PCU_ALT_0);
    HAL_PCU_SetAltMode(LED_PORT_ID, LED_ESP32, PCU_ALT_0);
    HAL_PCU_SetAltMode(LED_PORT_ID, LED_TEST1, PCU_ALT_0);
    HAL_PCU_SetAltMode(LED_PORT_ID, LED_TEST2, PCU_ALT_0);
    HAL_PCU_SetAltMode(LED_PORT_ID, LED_TEST3, PCU_ALT_0);
    
    /* Turn off all LEDs initially */
    LED_OFF(LED_DEBUG);
    LED_OFF(LED_SHT40);
    LED_OFF(LED_BATTERY);
    LED_OFF(LED_LIS2DE12);
    LED_OFF(LED_MIC);
    LED_OFF(LED_ESP32);
    LED_OFF(LED_TEST1);
    LED_OFF(LED_TEST2);
    LED_OFF(LED_TEST3);
    
    /* Configure I2C pins */
    HAL_PCU_SetAltMode(I2C_SCL_PORT, I2C_SCL_PIN, I2C_ALT_FUNCTION);
    HAL_PCU_SetAltMode(I2C_SDA_PORT, I2C_SDA_PIN, I2C_ALT_FUNCTION);
    
    /* Configure UART2 pins (ESP32-AT Module) */
    HAL_PCU_SetAltMode(UART2_PORT, UART2_RXD_PIN, UART2_ALT_FUNCTION);  // PA8 = UART2 RXD
    HAL_PCU_SetAltMode(UART2_PORT, UART2_TXD_PIN, UART2_ALT_FUNCTION);  // PA9 = UART2 TXD
}

/**********************************************************************
 * @brief       User Code Here
 * @param[in]   None
 * @return      None
 **********************************************************************/
void PRV_USER_Code(void)
{
    SysTick_Config(SystemCoreClock/TC_SYSTICK_1MS);
    GPIO_Config_Alt();
    
    /* 1. Debug Framework Init */
    if (DebugFramework_Init())
    {
        DebugFramework_PutsLine("[OK] DebugFramework Initialized");
        LED_ON(LED_DEBUG);
    }
    else
    {
        LED_Blink(LED_DEBUG, 5);
    }
    
        DebugFramework_PutsLine("\n\r\n\r");
    DebugFramework_PutsLine("############################################");
    DebugFramework_PutsLine("#        MINDBOARD TIREMO       #");
    DebugFramework_PutsLine("#              v2.0      #");
    DebugFramework_PutsLine("############################################\n\r");
    

    /* 2. LED Hardware Test */
    Sensor_LEDTest();

    /* 3. Sensor Test - initialize and verify all sensors */
    uint8_t sensorFails = Sensor_TestAll();
    if (sensorFails > 0)
    {
        DebugFramework_Printf("[WARN] %u sensor(s) failed init\n\r", sensorFails);
    }

    /* 4. ESP32 AT Test */
    DebugFramework_PutsLine("\n\r[TEST] ESP32-AT WiFi Module...");
    SYSTICK_Wait(2000);
    if (ESP32_Simple_AT_Test() == 0)
    {
        DebugFramework_PutsLine("[PASS] ESP32 OK");
        LED_ON(LED_ESP32);
    }
    else
    {
        DebugFramework_PutsLine("[FAIL] ESP32 NOT responding");
        LED_Blink(LED_ESP32, 3);
    }

    /* 5. MQTT Connect */
    DebugFramework_PutsLine("\n\r[MQTT] Connecting to broker...");
    MqttPort_ABOV_Init();
    uint8_t mqttConnected = (MQTT_ConnectBroker() == 0) ? 1 : 0;

    if (mqttConnected)
        DebugFramework_PutsLine("[MQTT] Ready to publish sensor data\n\r");
    else
        DebugFramework_PutsLine("[MQTT] Will retry each cycle\n\r");

    /* 6. Main Loop: Read -> Print -> Publish */
    uint32_t cycle = 0;

    while (1)
    {
        DebugFramework_PutsLine("\n\r========================================");
        DebugFramework_Printf("   CYCLE #%lu\n\r", cycle++);
        DebugFramework_PutsLine("========================================");

        /* (a) Read all sensors and print to terminal */
        SensorData_t *pData = Sensor_ReadAndPrint();

        /* (b) Publish to MQTT */
        if (mqttConnected)
        {
            DebugFramework_PutsLine("[MQTT] Data sending!");
            MQTT_PublishSensorData(pData);
            DebugFramework_PutsLine("[MQTT] Data sent!");
        }
        else
        {
            DebugFramework_PutsLine("[MQTT] Not connected, skipping publish");
            /* Try reconnect every cycle */
            mqttConnected = (MQTT_ConnectBroker() == 0) ? 1 : 0;
            if (mqttConnected)
            {
                MQTT_PublishSensorData(pData);
                DebugFramework_PutsLine("[MQTT] Reconnected & data sent!");
            }
        }

        /* (c) Wait before next cycle */
        DebugFramework_PutsLine("[INFO] Next cycle in 5 seconds...\n\r");
        SYSTICK_Wait(5000);
    }
}
