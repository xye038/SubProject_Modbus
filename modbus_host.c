#include "modbus_crc.h"
#include "modbus_host.h"

void ModbusHost_Send_Read(void(* DMA_UART_Send)(uint8_t*,uint16_t),
                          uint8_t add, uint8_t cmd, uint16_t startAddr,
                          uint16_t read_data_len, uint8_t *txBuffer)
{
    uint16_t crc;
    switch(cmd)
    {
      case 0x01:      //读1路或多路开关量线圈输出状态
      case 0x02:      //读1路或多路开关量状态输入
      case 0x03:      //读多路寄存器
          *txBuffer = add;
          *(txBuffer+1) = cmd;
          *(txBuffer+2) = (uint8_t)(startAddr>>0x08); //高8位在前
          *(txBuffer+3) = (uint8_t)(startAddr&0xff);
          *(txBuffer+4) = (uint8_t)(read_data_len>>0x08);  //高8位在前
          *(txBuffer+5) = (uint8_t)(read_data_len&0xff);
          crc = modbus_crc16(txBuffer,6);
          *(txBuffer+6) = (uint8_t)(crc&0xff);
          *(txBuffer+7) = (uint8_t)(crc>>8);
          DMA_UART_Send(txBuffer,8);
      break;
      default:
      break;
    }
}

void ModbusHost_Send_Write(void(* DMA_UART_Send)(uint8_t*,uint16_t),
                           uint8_t add, uint8_t cmd, uint16_t startAddr,
                           uint8_t write_data_len, uint8_t *txBuffer,
                           uint16_t *wirte_data)
{
    uint16_t crc;
    uint8_t i;
    switch(cmd)
    {
      case 0x05:     //写1路开关量输出
      case 0x06:     //写单路寄存器
          *txBuffer = add;
          *(txBuffer+1) = cmd;
          *(txBuffer+2) = (uint8_t)(startAddr>>0x08); //高8位在前
          *(txBuffer+3) = (uint8_t)(startAddr&0xff);
          *(txBuffer+4) = (uint8_t)(*wirte_data>>0x08);  //高8位在前
          *(txBuffer+5) = (uint8_t)(*wirte_data&0xff);
          crc = modbus_crc16(txBuffer,6);
          *(txBuffer+6) = (uint8_t)(crc&0xff);
          *(txBuffer+7) = (uint8_t)(crc>>8);
          DMA_UART_Send(txBuffer,8);
     break;
     case 0x10:      //写多路寄存器
         *txBuffer = add;
         *(txBuffer+1) = cmd;
         *(txBuffer+2) = (uint8_t)(startAddr>>0x08); //高8位在前
         *(txBuffer+3) = (uint8_t)(startAddr&0xff);
         *(txBuffer+4) = 0x00;
         *(txBuffer+5) = write_data_len;
         *(txBuffer+6) = write_data_len*2;
         for(i=0;i<write_data_len;i++)
         {
           *(txBuffer+7+2*i) = (uint8_t)(*wirte_data>>0x08);  //高8位在前
           *(txBuffer+8+2*i) = (uint8_t)(*wirte_data&0xff);
           wirte_data++;
         }
         crc = modbus_crc16(txBuffer,7+2*write_data_len);
         *(txBuffer+7+2*write_data_len) = (uint8_t)(crc&0xff);
         *(txBuffer+8+2*write_data_len) = (uint8_t)(crc>>8);
         DMA_UART_Send(txBuffer,9+2*write_data_len);
         break;
     default:
         break;
    }
}

uint16_t ModbusHost_Receive(uint8_t *rxBuffer,uint32_t length)
{
    uint16_t crc;
    uint16_t crc_data;
    uint16_t Read_Data;
    crc_data = (uint16_t)*(rxBuffer+length-1)<<8|*(rxBuffer+length-2);
    crc = modbus_crc16(rxBuffer,length-2);
    if(crc == crc_data)
    {
       switch(*(rxBuffer+1))
       {
        case 0x06:
            Read_Data = (uint16_t)*(rxBuffer+4)<<8|*(rxBuffer+5);
           return Read_Data;
        case 0x03:
            Read_Data = (uint16_t)*(rxBuffer+3)<<8|*(rxBuffer+4);
           return Read_Data;
       }
    }
    return 0xFFFF;
}

