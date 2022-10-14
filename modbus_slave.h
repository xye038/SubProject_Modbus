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
    uint16_t holdingReg_start_address;  //���ּĴ�����ʼ��ַ
    uint16_t holdingReg_end_address;  //���ּĴ���������ַ
    uint16_t comingReg_start_address;  //����Ĵ�����ʼ��ַ
    uint16_t comingReg_end_address;  //����Ĵ���������ַ
    uint16_t* holdingReg;    //���ּĴ������ݵ�ַ
    uint16_t* comingReg;     //����Ĵ������ݵ�ַ
    struct ModbusSlave_Reg *Next;
}ModbusSlave_Reg_t;

typedef struct{
    uint8_t Protocol;        //Э������ 0��RTU  1��TCP
    uint8_t slave_address;   //�豸��ַ
    uint8_t function_code;   //������
    ModbusSlave_Reg_t *Reg;  //�Ĵ����ṹ��
    uint16_t holdingReg_start_address;  //���ּĴ�����ʼ��ַ (���պ����������֮��ֵ�����ͺ�����)
    uint16_t comingReg_start_address;  //����Ĵ�����ʼ��ַ (���պ����������֮��ֵ�����ͺ�����)
    uint16_t* holdingReg;    //���ּĴ������ݵ�ַ (���պ����������֮��ֵ�����ͺ�����)
    uint16_t* comingReg;     //����Ĵ������ݵ�ַ  (���պ����������֮��ֵ�����ͺ�����)
    uint16_t length;          //�Ĵ�������
    uint8_t state;           //״̬    0������   1����ַ����  2����������� 3:���ݹ���
    uint8_t add_offset;      //��ַƫ����
    /*******MBAP����ͷ *************/
    uint16_t TransID;        //�������ʶ��
    uint16_t ProtoID;        //Э���ʶ��
    uint16_t DataLength;     //���ݳ���
    uint8_t  UnitID;         //��λ��ʶ�� �����豸��ַ
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
