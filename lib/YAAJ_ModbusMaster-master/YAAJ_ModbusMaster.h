#pragma once
/*

	YAAJ_ModbusMaster.h

	Forked from : https://github.com/4-20ma/ModbusMaster

	Arduino library for communicating with Modbus slaves over RS232/485 (via RTU protocol).

	https://4-20ma.io/ModbusMaster/index.html
	http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
	http://www.modbus.org/docs/PI_MBUS_300.pdf

	performances :
		
	master : STM32F103C (Blue/Black Pill)
	slaves : ATmega328P (Nano, Pro Mini, with SimpleModbusMaster v10))

	-> from 4800 to 256600 bauds

	STM32 can send up to 4Mbps (STM32 was not tested as slave)
	
	Library:: ModbusMaster

	Copyright:: 2009-2016 Doc Walker

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#include <Arduino.h>

class YAAJ_ModbusMaster
{
protected:
	// communication
	HardwareSerial* pSerial; // serial port
	uint32_t baudRate;
	uint8_t  slaveAddr;
	uint16_t timeout; // Modbus timeout [milliseconds]
	int8_t txEnablePin; // RE-DE MAX485

	// timing
	uint32_t T1_5; // inter character timeout
	uint32_t T3_5; // frame delay (silent interval)

	// buffers
#define MM_BUFFER_SIZE	64
	
	// buffer containing data to transmit to Modbus slave; set via SetSendBuffer()
	uint16_t txBuf[MM_BUFFER_SIZE];
	uint8_t txBufIdx;
	uint16_t txBufLen;
	
	// buffer to store Modbus slave response; read via GetReceiveBuffer()
	uint16_t rxBuf[MM_BUFFER_SIZE];
	uint8_t rxBufIdx;
	uint8_t rxBufLen;

	// slave register to which to write
	uint16_t regWriteAddr;
	uint16_t writeQty;
	
	// slave register from which to read
	uint16_t regReadAddr;
	uint16_t readQty; // quantity of words to read

public:
	// sets T1.5 and T3.5 and calls serial.begin()
	void begin(HardwareSerial& serial,
				uint32_t baudRate, SerialConfig config, int8_t rxPin, int8_t txPin, // see HardwareSerial::begin()
				uint8_t slaveAddr, int8_t TxEnablePin, uint16_t timeout);

	uint8_t getSlaveAddr();
	uint8_t setSlaveAddr(uint8_t newAddr);
		
	uint16_t getRxBuf(uint8_t index);
	void clearRxBuf();
	
	uint8_t setTxBuf(uint8_t index, uint16_t word);
	void clearTxBuf();

	void beginTransmission(uint16_t regWriteAddr);

	void sendBit(bool bit);
	void send(uint8_t byte);
	void send(uint16_t word);
	void send(uint32_t dword);

	uint8_t available(void);
	uint16_t receive(void);

public:
	uint8_t F1_ReadCoils(uint16_t regAddr, uint16_t count);
	uint8_t F2_ReadDiscreteInputs(uint16_t regAddr, uint16_t count);
	uint8_t F3_ReadMultipleHoldingRegisters(uint16_t regAddr, uint16_t count);
	uint8_t F4_ReadInputRegisters(uint16_t regAddr, uint8_t count);
	uint8_t F5_WriteSingleCoil(uint16_t regAddr, uint8_t state);
	uint8_t F6_WriteSingleRegister(uint16_t regAddr, uint16_t count);
	uint8_t F15_WriteMultipleCoils(uint16_t regAddr, uint16_t count);
	uint8_t F15_WriteMultipleCoils();
	uint8_t F16_WriteMultipleHoldingRegisters(uint16_t regAddr, uint16_t count);
	uint8_t F16_WriteMultipleHoldingRegisters();
	uint8_t F22_MaskWriteRegister(uint16_t regAddr, uint16_t andMask, uint16_t orMask);
	uint8_t F23_ReadWriteMultipleRegisters(uint16_t regReadAddr, uint16_t readQty,
													uint16_t regWriteAddr, uint16_t writeQty);
	uint8_t F23_ReadWriteMultipleRegisters(uint16_t regAddr, uint16_t count);

protected:
	// master function that conducts Modbus transactions
	uint8_t ModbusMasterTransaction(uint8_t function);
};

// Modbus RTU exception codes https://4-20ma.io/ModbusMaster/group__constant.html

typedef enum
{
	MODBUS_SUCCESS = 0x00,
	MODBUS_ILLEGAL_FUNCTION,
	MODBUS_ILLEGAL_DATA_ADDRESS,
	MODBUS_ILLEGAL_DATA_VALUE,
	MODBUS_SLAVE_DEVICE_FAILURE,
	MODBUS_ACKNOWLEDGE,
	MODBUS_SLAVE_DEVICE_BUSY,
	MODBUS_NEGATIVE_ACKNOWLEDGE,
	MODBUS_MEMORY_PARITY_ERROR,

	MODBUS_GATEWAY_PATH_UNAVAILABLE = 0x0A,
	MODBUS_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND,

	MODBUS_INVALID_SLAVE_ID = 0xE0,
	MODBUS_INVALID_FUNCTION,
	MODBUS_RESPONSE_TIMED_OUT,
	MODBUS_INVALID_CRC
} mobusErrorCode;


