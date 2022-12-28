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
    uint16_t holdingReg_start_address;  //���ּĴ�����ʼ��ַ
    uint16_t holdingReg_end_address;  //���ּĴ���������ַ
    uint16_t comingReg_start_address;  //����Ĵ�����ʼ��ַ
    uint16_t comingReg_end_address;  //����Ĵ���������ַ
    volatile uint16_t* holdingReg;    //���ּĴ������ݵ�ַ
    volatile uint16_t* comingReg;     //����Ĵ������ݵ�ַ
    struct ModbusSlave_Reg *Next;
}ModbusSlave_Reg_t;

typedef struct{
    uint8_t slave_address;   //�豸��ַ
    uint8_t function_code;   //������
    ModbusSlave_Reg_t *Reg;  //�Ĵ����ṹ��
    uint16_t holdingReg_start_address;  //���ּĴ�����ʼ��ַ (���պ����������֮��ֵ�����ͺ�����)
    uint16_t comingReg_start_address;  //����Ĵ�����ʼ��ַ (���պ����������֮��ֵ�����ͺ�����)
    volatile uint16_t* holdingReg;    //���ּĴ������ݵ�ַ (���պ����������֮��ֵ�����ͺ�����)
    volatile uint16_t* comingReg;     //����Ĵ������ݵ�ַ  (���պ����������֮��ֵ�����ͺ�����)
    uint16_t length;          //�Ĵ�������
    uint8_t state;           //״̬    0������   1����ַ����  2����������� 3:���ݹ���
    uint8_t add_offset;      //��ַƫ����
}ModbusSlave_Str;

void ModbusSlave_Reg_Create(ModbusSlave_Reg_t *ModbusSlave_Reg,
                            uint16_t holdingReg_start_address, uint16_t holdingReg_end_address,
                            uint16_t comingReg_start_address, uint16_t comingReg_end_address,
                            volatile uint16_t* holdingReg, volatile uint16_t* comingReg);

void ModbusSlave_Struct_Init(ModbusSlave_Str *SlaveStr, uint8_t address);

void ModbusSlave_Receive_DataProcess(ModbusSlave_Str *modbus_slave, uint8_t *rxBuffer, uint32_t length);

uint16_t ModbusSlave_Send_DataProcess(ModbusSlave_Str *modbus_slave,uint8_t *txBuffer);

#endif
