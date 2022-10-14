#ifndef __MODBUS_SLAVE_H__
#define __MODBUS_SLAVE_H__

#define  NoError              0
#define  AddError             1
#define  FunCodeError         2
#define  LenError             3
#define  MBAPError            0x0A

#define Modbus_RTU            0
#define Modbus_TCP            1

typedef struct ModbusSlave_Reg{
    uint16_t holdingReg_start_address;  //保持寄存器起始地址
    uint16_t holdingReg_end_address;  //保持寄存器结束地址
    uint16_t comingReg_start_address;  //输入寄存器起始地址
    uint16_t comingReg_end_address;  //输入寄存器结束地址
    uint16_t* holdingReg;    //保持寄存器数据地址
    uint16_t* comingReg;     //输入寄存器数据地址
    struct ModbusSlave_Reg *Next;
}ModbusSlave_Reg_t;

typedef struct{
    uint8_t Protocol;        //协议类型 0：RTU  1：TCP
    uint8_t slave_address;   //设备地址
    uint8_t function_code;   //功能码
    ModbusSlave_Reg_t *Reg;  //寄存器结构体
    uint16_t holdingReg_start_address;  //保持寄存器起始地址 (接收函数处理完毕之后赋值给发送函数用)
    uint16_t comingReg_start_address;  //输入寄存器起始地址 (接收函数处理完毕之后赋值给发送函数用)
    uint16_t* holdingReg;    //保持寄存器数据地址 (接收函数处理完毕之后赋值给发送函数用)
    uint16_t* comingReg;     //输入寄存器数据地址  (接收函数处理完毕之后赋值给发送函数用)
    uint16_t length;          //寄存器数量
    uint8_t state;           //状态    0：正常   1：地址错误  2：功能码错误 3:数据过长
    uint8_t add_offset;      //地址偏移量
    /*******MBAP报文头 *************/
    uint16_t TransID;        //事务处理标识符
    uint16_t ProtoID;        //协议标识符
    uint16_t DataLength;     //数据长度
    uint8_t  UnitID;         //单位标识符 就是设备地址
}ModbusSlave_Str;

void ModbusSlave_Reg_Create(ModbusSlave_Reg_t *ModbusSlave_Reg,
                            uint16_t holdingReg_start_address, uint16_t holdingReg_end_address,
                            uint16_t comingReg_start_address, uint16_t comingReg_end_address,
                            uint16_t* holdingReg, uint16_t* comingReg);

void ModbusSlave_Struct_Init(ModbusSlave_Str *SlaveStr, uint8_t ProtoType,
                             uint8_t address, uint16_t Protocol_ID);

void ModbusSlave_Receive(ModbusSlave_Str *modbus_slave, uint8_t *rxBuffer, uint32_t length);

void ModbusSlave_Send(void(* DMA_UART_Send)(uint8_t*,uint16_t),
                     ModbusSlave_Str *modbus_slave,uint8_t *txBuffer);

void ModbusSlave_TCP_Receive(ModbusSlave_Str *modbus_slave, uint8_t *rxBuffer, uint32_t length);

uint8_t ModbusSlave_TCP_Send(uint8_t(* DMA_UART_Send)(uint8_t*,uint16_t),
                     ModbusSlave_Str *modbus_slave,uint8_t *txBuffer);

#endif
