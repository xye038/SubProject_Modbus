#ifndef __MODBUS_CRC_H__
#define __MODBUS_CRC_H__

#include "ch32v30x.h"

uint16_t modbus_crc16( uint8_t * pFrame, uint16_t len );

#endif
