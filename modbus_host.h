#ifndef __MODBUS_HOST_H__
#define __MODBUS_HOST_H__


void ModbusHost_Send_Read(void(* DMA_UART_Send)(uint8_t*,uint16_t),
                          uint8_t add, uint8_t cmd, uint16_t startAddr,
                          uint16_t read_data_len, uint8_t *txBuffer);

void ModbusHost_Send_Write(void(* DMA_UART_Send)(uint8_t*,uint16_t),
                           uint8_t add, uint8_t cmd, uint16_t startAddr,
                           uint8_t write_data_len, uint8_t *txBuffer,
                           uint16_t *wirte_data);

uint16_t ModbusHost_Receive(uint8_t *rxBuffer,uint32_t length);


#endif
