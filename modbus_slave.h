#ifndef __MODBUS_SLAVE_H__
#define __MODBUS_SLAVE_H__

#include "modbus_crc.h"

#define  NoError              0
#define  AddError             1
#define  FunCodeError         2
#define  LenError             3
#define  NoACK                4
#define  MBAPError            0x0A

#define NoAddr                0xFFFF

typedef struct ModbusSlave_Reg{
    uint16_t holdingReg_start_address;  //保持寄存器起始地址
    uint16_t holdingReg_end_address;  //保持寄存器结束地址
    uint16_t comingReg_start_address;  //输入寄存器起始地址
    uint16_t comingReg_end_address;  //输入寄存器结束地址
    volatile uint16_t* holdingReg;    //保持寄存器数据地址
    volatile uint16_t* comingReg;     //输入寄存器数据地址
    struct ModbusSlave_Reg *Next;
}ModbusSlave_Reg_t;

typedef struct{
    uint8_t slave_address;   //设备地址
    uint8_t function_code;   //功能码
    ModbusSlave_Reg_t *Reg;  //寄存器结构体
    uint16_t holdingReg_start_address;  //保持寄存器起始地址 (接收函数处理完毕之后赋值给发送函数用)
    uint16_t comingReg_start_address;  //输入寄存器起始地址 (接收函数处理完毕之后赋值给发送函数用)
    volatile uint16_t* holdingReg;    //保持寄存器数据地址 (接收函数处理完毕之后赋值给发送函数用)
    volatile uint16_t* comingReg;     //输入寄存器数据地址  (接收函数处理完毕之后赋值给发送函数用)
    uint16_t length;          //寄存器数量
    uint8_t state;           //状态    0：正常   1：地址错误  2：功能码错误 3:数据过长
    uint8_t add_offset;      //地址偏移量
}ModbusSlave_Str;

void ModbusSlave_Reg_Create(ModbusSlave_Reg_t *ModbusSlave_Reg,
                            uint16_t holdingReg_start_address, uint16_t holdingReg_end_address,
                            uint16_t comingReg_start_address, uint16_t comingReg_end_address,
                            volatile uint16_t* holdingReg, volatile uint16_t* comingReg);

void ModbusSlave_Struct_Init(ModbusSlave_Str *SlaveStr, uint8_t address);

void ModbusSlave_Receive_DataProcess(ModbusSlave_Str *modbus_slave, uint8_t *rxBuffer, uint32_t length);

uint16_t ModbusSlave_Send_DataProcess(ModbusSlave_Str *modbus_slave,uint8_t *txBuffer);

#endif
