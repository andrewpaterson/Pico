#ifndef __TESTER_PINS_H__
#define __TESTER_PINS_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "GeneralPins.h"
#include "PrimitiveTypes.h"
#include "StringHelper.h"


#define ENABLE_GPIO             28
#define ONBOARD_LED             25

#define PWR_12V                 0
#define PWR_5V_A                1
#define PWR_5V_B                2
#define PWR_5V_C                3
#define PWR_GND_A               4
#define PWR_GND_B               5
#define PWR_GND_C               6
#define PWR_GND_D               7

#define DATA_LINE_0             17
#define DATA_LINE_1             18
#define DATA_LINE_2             19
#define DATA_LINE_3             20
#define DATA_LINE_4             21
#define DATA_LINE_5             22
#define DATA_LINE_6             26
#define DATA_LINE_7             27


#define ADDR_LINE_ENABLE        8
#define ADDR_LINE_0             9   //  1
#define ADDR_LINE_1             10  //  2
#define ADDR_LINE_2             11  //  4
#define ADDR_LINE_3             12  //  8
#define ADDR_LINE_4             13  // 10  (high - write, low - read)
#define ADDR_LINE_5             14  // 20


#define READ_IN                 15
#define WRITE_OUT               16

//Addresses
#define ADDRESS_READ_GPIO_56_63     0x00
#define ADDRESS_READ_GPIO_48_55     0x01
#define ADDRESS_READ_GPIO_40_47     0x02
#define ADDRESS_READ_GPIO_32_39     0x03
#define ADDRESS_READ_GPIO_24_31     0x04
#define ADDRESS_READ_GPIO_16_23     0x05
#define ADDRESS_READ_GPIO__8_15     0x06
#define ADDRESS_READ_GPIO__0__7     0x07
#define ADDRESS_READ_GPIO_120_127   0x08
#define ADDRESS_READ_GPIO_112_119   0x09
#define ADDRESS_READ_GPIO_104_111   0x0A
#define ADDRESS_READ_GPIO_96_103    0x0B
#define ADDRESS_READ_GPIO_88_95     0x0C
#define ADDRESS_READ_GPIO_80_87     0x0D
#define ADDRESS_READ_GPIO_72_79     0x0E
#define ADDRESS_READ_GPIO_64_71     0x0F

extern uint8 aiReadAddresses[16];

#define ADDRESS_WRITE_GPIO_56_63    0x10
#define ADDRESS_WRITE_GPIO_48_55    0x11
#define ADDRESS_WRITE_GPIO_40_47    0x12
#define ADDRESS_WRITE_GPIO_32_39    0x13
#define ADDRESS_WRITE_GPIO_24_31    0x14
#define ADDRESS_WRITE_GPIO_16_23    0x15
#define ADDRESS_WRITE_GPIO__8_15    0x16
#define ADDRESS_WRITE_GPIO__0__7    0x17

#define ADDRESS_OUTPUT_GPIO_56_63   0x18
#define ADDRESS_OUTPUT_GPIO_48_55   0x19
#define ADDRESS_OUTPUT_GPIO_40_47   0x1A
#define ADDRESS_OUTPUT_GPIO_32_39   0x1B
#define ADDRESS_OUTPUT_GPIO_24_31   0x1C
#define ADDRESS_OUTPUT_GPIO_16_23   0x1D
#define ADDRESS_OUTPUT_GPIO__8_15   0x1E
#define ADDRESS_OUTPUT_GPIO__0__7   0x1F


void InitPicoPins(void);

void SetDataToInput(void);
void SetDataToOutput(void);

uint8 ReverseByte(uint8 uiByte);
uint32 MakeAddress(uint8 uiAddress, bool bEnable);
void WriteData(uint8 uiAddress, uint8 uiData);
void PulseWriteData(void);
void PulseReadData(void);
uint8 ReadData(uint8 uiAddress);
void SetAllToOutput(uint8 uiValue);
void SetAllToData(uint8 uiValue);


#endif // __TESTER_PINS_H__

