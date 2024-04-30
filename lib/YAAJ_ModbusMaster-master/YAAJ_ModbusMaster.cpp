/*
	YAAJ_ModbusMaster.cpp

	NOTE: Small changes were made to the begin function for the ShearStressBioreactor project.

	Forked from : https://github.com/4-20ma/ModbusMaster

	Arduino library for communicating with Modbus slaves over RS232/485 (via RTU protocol).

	https://4-20ma.io/ModbusMaster/index.html
	http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
	http://www.modbus.org/docs/PI_MBUS_300.pdf

	performances :

	master : STM32F103C (Blue/Black Pill)
	slaves : ATmega328P (Nano, Pro Mini, with SimpleModbusMaster v10))

	-> from 4800 to 256600 bauds

	STM32 can send up to 4Mbps (not tested as slave)

	ModbusMaster.cpp - Arduino library for communicating with Modbus slaves
	over RS232/485 (via RTU protocol).

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

#include "YAAJ_ModbusMaster.h"

#define F1_READ_COILS							0x01
#define F2_READ_DISCRETE_INPUTS					0x02
#define F3_READ_HOLDING_REGISTERS				0x03
#define F4_READ_INPUT_REGISTERS					0x04
#define F5_MBM_WRITE_SINGLE_COIL				0x05
#define F6_WRITE_SINGLE_REGISTER				0x06
#define F15_WRITE_MULTIPLE_COILS				0x0F
#define F16_WRITE_MULTIPLE_REGISTERS			0x10
#define F22_MASK_WRITE_REGISTER					0x16
#define F23_READ_WRITE_MULTIPLE_REGISTERS		0x17

static uint16_t crc16_update(uint16_t crc, uint8_t a)
{
	int i;

	crc ^= a;
	for (i = 0; i < 8; ++i)
	{
		if (crc & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc = crc >> 1;
	}

	return crc;
}

inline static uint16_t lowWord(uint32_t dword)
{
	return (uint16_t)(dword & 0xFFFF);
}

inline static uint16_t highWord(uint32_t dword)
{
	return (uint16_t)(dword >> 16);
}

uint8_t YAAJ_ModbusMaster::getSlaveAddr()
{
	return slaveAddr;
}

uint8_t YAAJ_ModbusMaster::setSlaveAddr(uint8_t newAddr)
{
	uint8_t old = slaveAddr;
	slaveAddr = newAddr;
	return old;
}

void YAAJ_ModbusMaster::begin(HardwareSerial& serial, uint32_t baud, SerialConfig config, int8_t rxPin, int8_t txPin,
								uint8_t _slaveAddr, int8_t _TxEnablePin, uint16_t _timeout)
{
	pSerial = &serial;
	txEnablePin = _TxEnablePin;
	timeout = _timeout;
	txBufIdx = 0;
	txBufLen = 0;
	slaveAddr = _slaveAddr;
	baudRate = baud;

	pinMode(txEnablePin, OUTPUT);
	digitalWrite(txEnablePin, LOW);
	pSerial->begin(baudRate, config, rxPin, txPin);

	// calculate inter character timeout and frame delay (silent interval)
	if (baudRate > 19200)
	{
		T1_5 = 750; // 750 us
		T3_5 = 1750; // 1750 us
	}
	else
	{
		T1_5 = 16500000 / baudRate; // 1 packet = 11 bits
		T3_5 = 38500000 / baudRate; // 1 packet = 11 bits
	}
}

uint16_t YAAJ_ModbusMaster::getRxBuf(uint8_t index)
{
	return index < MM_BUFFER_SIZE ? rxBuf[index] : 0xFFFF;
}

void YAAJ_ModbusMaster::clearRxBuf()
{
	for (uint8_t i = 0; i < MM_BUFFER_SIZE; i++)
		rxBuf[i] = 0;
}

uint8_t YAAJ_ModbusMaster::setTxBuf(uint8_t index, uint16_t word)
{
	if (index < MM_BUFFER_SIZE)
	{
		txBuf[index] = word;

		return MODBUS_SUCCESS;
	}
	else
		return MODBUS_ILLEGAL_DATA_ADDRESS;
}

void YAAJ_ModbusMaster::clearTxBuf()
{
	for (uint8_t i = 0; i < MM_BUFFER_SIZE; i++)
		txBuf[i] = 0;
}

inline void YAAJ_ModbusMaster::beginTransmission(uint16_t _writeRegAddress)
{
	regWriteAddr = _writeRegAddress;
	txBufIdx = 0;
	txBufLen = 0;
}

void YAAJ_ModbusMaster::sendBit(bool bit)
{
	uint8_t txBitIndex = txBufLen % 16;

	if ((txBufLen >> 4) < MM_BUFFER_SIZE)
	{
		if (txBitIndex == 0)
			txBuf[txBufIdx] = 0;

		bitWrite(txBuf[txBufIdx], txBitIndex, bit);
		txBufLen++;
		txBufIdx = txBufLen >> 4;
	}
}

void YAAJ_ModbusMaster::send(uint8_t byte)
{
	send(word(byte));
}

void YAAJ_ModbusMaster::send(uint16_t word)
{
	if (txBufIdx < MM_BUFFER_SIZE)
	{
		txBuf[txBufIdx++] = word;
		txBufLen = txBufIdx << 4;
	}
}

void YAAJ_ModbusMaster::send(uint32_t dword)
{
	send(lowWord(dword));
	send(highWord(dword));
}

uint8_t YAAJ_ModbusMaster::available(void)
{
	return rxBufLen - rxBufIdx;
}

uint16_t YAAJ_ModbusMaster::receive(void)
{
	return rxBufIdx < rxBufLen ? rxBuf[rxBufIdx++] : 0xFFFF;
}

uint8_t YAAJ_ModbusMaster::F16_WriteMultipleHoldingRegisters(uint16_t regAddr, uint16_t count)
{
	regWriteAddr = regAddr;
	writeQty = count;

	return ModbusMasterTransaction(F16_WRITE_MULTIPLE_REGISTERS);
}

uint8_t YAAJ_ModbusMaster::F1_ReadCoils(uint16_t regAddr, uint16_t count)
{
	regReadAddr = regAddr;
	readQty = count;
	
	return ModbusMasterTransaction(F1_READ_COILS);
}

uint8_t YAAJ_ModbusMaster::F2_ReadDiscreteInputs(uint16_t regAddr, uint16_t count)
{
	regReadAddr = regAddr;
	readQty = count;
	
	return ModbusMasterTransaction(F2_READ_DISCRETE_INPUTS);
}

uint8_t YAAJ_ModbusMaster::F3_ReadMultipleHoldingRegisters(uint16_t regAddr, uint16_t count)
{
	regReadAddr = regAddr;
	readQty = count;

	return ModbusMasterTransaction(F3_READ_HOLDING_REGISTERS);
}

uint8_t YAAJ_ModbusMaster::F4_ReadInputRegisters(uint16_t regAddr, uint8_t count)
{
	regReadAddr = regAddr;
	readQty = count;

	return ModbusMasterTransaction(F4_READ_INPUT_REGISTERS);
}

uint8_t YAAJ_ModbusMaster::F5_WriteSingleCoil(uint16_t regAddr, uint8_t state)
{
	regWriteAddr = regAddr;
	writeQty = state ? 0xFF00 : 0x0000;

	return ModbusMasterTransaction(F5_MBM_WRITE_SINGLE_COIL);
}

uint8_t YAAJ_ModbusMaster::F6_WriteSingleRegister(uint16_t regAddr, uint16_t value)
{
	regWriteAddr = regAddr;
	writeQty = 0;
	txBuf[0] = value;

	return ModbusMasterTransaction(F6_WRITE_SINGLE_REGISTER);
}

uint8_t YAAJ_ModbusMaster::F15_WriteMultipleCoils(uint16_t regAddr, uint16_t count)
{
	regWriteAddr = regAddr;
	writeQty = count;

	return ModbusMasterTransaction(F15_WRITE_MULTIPLE_COILS);
}

uint8_t YAAJ_ModbusMaster::F15_WriteMultipleCoils()
{
	writeQty = txBufLen;

	return ModbusMasterTransaction(F15_WRITE_MULTIPLE_COILS);
}

// new version based on Wire.h
uint8_t YAAJ_ModbusMaster::F16_WriteMultipleHoldingRegisters()
{
	writeQty = txBufIdx;

	return ModbusMasterTransaction(F16_WRITE_MULTIPLE_REGISTERS);
}

uint8_t YAAJ_ModbusMaster::F22_MaskWriteRegister(uint16_t regAddr, uint16_t andMask, uint16_t orMask)
{
	regWriteAddr = regAddr;
	txBuf[0] = andMask;
	txBuf[1] = orMask;

	return ModbusMasterTransaction(F22_MASK_WRITE_REGISTER);
}

uint8_t YAAJ_ModbusMaster::F23_ReadWriteMultipleRegisters(uint16_t _regReadAddr, uint16_t _readCount,
																uint16_t _regWriteAddr, uint16_t _writeCount)
{
	regReadAddr = _regReadAddr;
	readQty = _readCount;
	regWriteAddr = _regWriteAddr;
	writeQty = _writeCount;

	return ModbusMasterTransaction(F23_READ_WRITE_MULTIPLE_REGISTERS);
}

uint8_t YAAJ_ModbusMaster::F23_ReadWriteMultipleRegisters(uint16_t readAddr, uint16_t count)
{
	regReadAddr = readAddr;
	readQty = count;
	writeQty = txBufIdx;

	return ModbusMasterTransaction(F23_READ_WRITE_MULTIPLE_REGISTERS);
}

uint8_t YAAJ_ModbusMaster::ModbusMasterTransaction(uint8_t function)
{
	uint8_t count = 0;
	uint8_t i = 0;

	// ADU = Application Data Unit = address + PDU + error check
	// PDU = Protocol Data Unit = function code + data
	uint8_t modbusADU[256];
	uint8_t modbusADUSize = 0;

	uint32_t startTime;
	uint8_t bytesLeft = 8;
	uint8_t status = MODBUS_SUCCESS;

	// assemble Modbus Request Application Data Unit
	modbusADU[modbusADUSize++] = slaveAddr;
	modbusADU[modbusADUSize++] = function;

	switch (function)
	{
	case F1_READ_COILS:
	case F2_READ_DISCRETE_INPUTS:
	case F3_READ_HOLDING_REGISTERS:
	case F4_READ_INPUT_REGISTERS:
	case F23_READ_WRITE_MULTIPLE_REGISTERS:
		modbusADU[modbusADUSize++] = highByte(regReadAddr);
		modbusADU[modbusADUSize++] = lowByte(regReadAddr);
		modbusADU[modbusADUSize++] = highByte(readQty);
		modbusADU[modbusADUSize++] = lowByte(readQty);
		break;
	}

	switch (function)
	{
	case F5_MBM_WRITE_SINGLE_COIL:
	case F6_WRITE_SINGLE_REGISTER:
	case F15_WRITE_MULTIPLE_COILS:
	case F16_WRITE_MULTIPLE_REGISTERS:
	case F22_MASK_WRITE_REGISTER:
	case F23_READ_WRITE_MULTIPLE_REGISTERS:
		modbusADU[modbusADUSize++] = highByte(regWriteAddr);
		modbusADU[modbusADUSize++] = lowByte(regWriteAddr);
		break;
	}

	switch (function)
	{
	case F5_MBM_WRITE_SINGLE_COIL:
		modbusADU[modbusADUSize++] = highByte(writeQty);
		modbusADU[modbusADUSize++] = lowByte(writeQty);
		break;

	case F6_WRITE_SINGLE_REGISTER:
		modbusADU[modbusADUSize++] = highByte(txBuf[0]);
		modbusADU[modbusADUSize++] = lowByte(txBuf[0]);
		break;

	case F15_WRITE_MULTIPLE_COILS:
		modbusADU[modbusADUSize++] = highByte(writeQty);
		modbusADU[modbusADUSize++] = lowByte(writeQty);
		count = writeQty % 8 ? (writeQty >> 3) + 1 : writeQty >> 3;
		modbusADU[modbusADUSize++] = count;

		for (i = 0; i < count; i++)
		{
			switch (i % 2)
			{
			case 0: // i is even
				modbusADU[modbusADUSize++] = lowByte(txBuf[i >> 1]);
				break;

			case 1: // i is odd
				modbusADU[modbusADUSize++] = highByte(txBuf[i >> 1]);
				break;
			}
		}
		break;

	case F16_WRITE_MULTIPLE_REGISTERS:
	case F23_READ_WRITE_MULTIPLE_REGISTERS:

		modbusADU[modbusADUSize++] = highByte(writeQty);
		modbusADU[modbusADUSize++] = lowByte(writeQty);
		modbusADU[modbusADUSize++] = lowByte(writeQty << 1);

		for (i = 0; i < lowByte(writeQty); i++)
		{
			modbusADU[modbusADUSize++] = highByte(txBuf[i]);
			modbusADU[modbusADUSize++] = lowByte(txBuf[i]);
		}
		break;

	case F22_MASK_WRITE_REGISTER:
		modbusADU[modbusADUSize++] = highByte(txBuf[0]);
		modbusADU[modbusADUSize++] = lowByte(txBuf[0]);
		modbusADU[modbusADUSize++] = highByte(txBuf[1]);
		modbusADU[modbusADUSize++] = lowByte(txBuf[1]);
		break;
	}

	// append CRC
	uint16_t CRC = 0xFFFF;
	
	for (i = 0; i < modbusADUSize; i++)
		CRC = crc16_update(CRC, modbusADU[i]);

	modbusADU[modbusADUSize++] = lowByte(CRC);
	modbusADU[modbusADUSize++] = highByte(CRC);
	modbusADU[modbusADUSize] = 0;

	// flush receive buffer before transmitting request
	while (pSerial->read() != -1);

	// -----------------------------------
	// enable transmission
	// -----------------------------------
	digitalWrite(txEnablePin, HIGH);
	// -----------------------------------

	for (i = 0; i < modbusADUSize; i++)
		pSerial->write(modbusADU[i]);

	modbusADUSize = 0;
	pSerial->flush();    // flush transmit buffer

	// -----------------------------------
	// - inter frame delay (silent interval) 
	// - disable transmision
	delayMicroseconds(T3_5);
	digitalWrite(txEnablePin, LOW);
	// -----------------------------------

	// loop until we run out of time or bytes, or an error occurs
	startTime = millis();
	while (bytesLeft && !status)
	{
		if (pSerial->available())
		{
			modbusADU[modbusADUSize++] = pSerial->read();
			bytesLeft--;
		}

		// -----------------------------------
		// inter character timeout
		// -----------------------------------
		delayMicroseconds(T1_5);
		// -----------------------------------

		// evaluate slave ID, function code once enough bytes have been read
		if (modbusADUSize == 5)
		{
			// verify response is for correct Modbus slave
			if (modbusADU[0] != slaveAddr)
			{
				status = MODBUS_INVALID_SLAVE_ID;
				break;
			}

			// verify response is for correct Modbus function code (mask exception bit 7)
			if ((modbusADU[1] & 0x7F) != function)
			{
				status = MODBUS_INVALID_FUNCTION;
				break;
			}

			// check whether Modbus exception occurred; return Modbus Exception Code
			if (bitRead(modbusADU[1], 7))
			{
				status = modbusADU[2];
				break;
			}

			// evaluate returned Modbus function code
			switch (modbusADU[1])
			{
			case F1_READ_COILS:
			case F2_READ_DISCRETE_INPUTS:
			case F3_READ_HOLDING_REGISTERS:
			case F4_READ_INPUT_REGISTERS:
			case F23_READ_WRITE_MULTIPLE_REGISTERS:
				bytesLeft = modbusADU[2];
				break;

			case F5_MBM_WRITE_SINGLE_COIL:
			case F6_WRITE_SINGLE_REGISTER:
			case F15_WRITE_MULTIPLE_COILS:
			case F16_WRITE_MULTIPLE_REGISTERS:
				bytesLeft = 3;
				break;

			case F22_MASK_WRITE_REGISTER:
				bytesLeft = 5;
				break;
			}
		}

		if ((millis() - startTime) > timeout)
			status = MODBUS_RESPONSE_TIMED_OUT;
	}

	// verify response is large enough to inspect further
	if (!status && modbusADUSize >= 5)
	{
		// calculate CRC
		uint16_t CRC = 0xFFFF;

		for (i = 0; i < modbusADUSize - 2; i++)
			CRC = crc16_update(CRC, modbusADU[i]);

		// verify CRC
		if (!status && (lowByte(CRC) != modbusADU[modbusADUSize - 2] ||
			highByte(CRC) != modbusADU[modbusADUSize - 1]))
		{
			status = MODBUS_INVALID_CRC;
		}
	}

	// disassemble ADU into words
	if (!status)
	{
		// evaluate returned Modbus function code
		switch (modbusADU[1])
		{
		case F1_READ_COILS:
		case F2_READ_DISCRETE_INPUTS:

			// load bytes into word; response bytes are ordered L, H, L, H, ...

			for (i = 0; i < modbusADU[2] >> 1; i++)
			{
				if (i < MM_BUFFER_SIZE)
					rxBuf[i] = word(modbusADU[(i << 1) + 4], modbusADU[(i << 1) + 3]);

				rxBufLen = i;
			}

			// in the event of an odd number of bytes, load last byte into zero-padded word
			if (modbusADU[2] % 2)
			{
				if (i < MM_BUFFER_SIZE)
					rxBuf[i] = word(0, modbusADU[(i << 1) + 3]);

				rxBufLen = i + 1;
			}
			break;

		case F3_READ_HOLDING_REGISTERS:
		case F4_READ_INPUT_REGISTERS:
		case F23_READ_WRITE_MULTIPLE_REGISTERS:
			// load bytes into word; response bytes are ordered H, L, H, L, ...
			for (uint8_t i = 0; i < modbusADU[2] >> 1; i++)
			{
				if (i < MM_BUFFER_SIZE)
					rxBuf[i] = word(modbusADU[(i << 1) + 3], modbusADU[(i << 1) + 4]);

				rxBufLen = i;
			}
			break;
		}
	}

	txBufIdx = 0;
	txBufLen = 0;
	rxBufIdx = 0;

	return status;
}
