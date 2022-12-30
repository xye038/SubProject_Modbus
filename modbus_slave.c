#include "modbus_slave.h"

static ModbusSlave_Reg_t *Head_Reg = NULL;
static void Add_ModbusSlave_Reg(ModbusSlave_Reg_t *ModbusSlave_Reg);

void ModbusSlave_Reg_Create(ModbusSlave_Reg_t *ModbusSlave_Reg,
                            uint16_t holdingReg_start_address, uint16_t holdingReg_end_address,
                            uint16_t comingReg_start_address, uint16_t comingReg_end_address,
                            volatile uint16_t* holdingReg, volatile uint16_t* comingReg)
{
    ModbusSlave_Reg->holdingReg_start_address = holdingReg_start_address;
    ModbusSlave_Reg->holdingReg_end_address = holdingReg_end_address;
    ModbusSlave_Reg->comingReg_start_address = comingReg_start_address;
    ModbusSlave_Reg->comingReg_end_address = comingReg_end_address;
    ModbusSlave_Reg->holdingReg = holdingReg;
    ModbusSlave_Reg->comingReg = comingReg;
    Add_ModbusSlave_Reg(ModbusSlave_Reg);
 }

static void Add_ModbusSlave_Reg(ModbusSlave_Reg_t *ModbusSlave_Reg)
{
    ModbusSlave_Reg->Next = Head_Reg;
    Head_Reg = ModbusSlave_Reg;
}

void ModbusSlave_Struct_Init(ModbusSlave_Str *SlaveStr,uint8_t address)
{
    SlaveStr->slave_address = address;
    SlaveStr->function_code = 0x00;
    SlaveStr->Reg = Head_Reg;
    SlaveStr->length = 0;
    SlaveStr->state = 0;
    SlaveStr->add_offset = 0;
}


void ModbusSlave_Receive_DataProcess(ModbusSlave_Str *modbus_slave, uint8_t *rxBuffer, uint32_t length)
{
    uint8_t i;
    uint16_t crc;
    uint16_t crc_data;
    uint16_t address;
    ModbusSlave_Reg_t *pass_reg;
    if((*rxBuffer != modbus_slave->slave_address))
    {
         modbus_slave->state = NoACK;
         return;
    }
    crc_data = (uint16_t)*(rxBuffer+length-2)<<8|*(rxBuffer+length-1);
    crc = modbus_crc16(rxBuffer,length-2);
    if(crc == crc_data)
    {
       switch(*(rxBuffer+1))
       {
        case 0x06:   //写单个保持寄存器
            address = *(rxBuffer+2)<<8|*(rxBuffer+3);
            modbus_slave->function_code = 0x06;
            for(pass_reg=Head_Reg; pass_reg!=NULL; pass_reg=pass_reg->Next)
            {
                if((address>=pass_reg->holdingReg_start_address)&&(address<=pass_reg->holdingReg_end_address))
                {
                    modbus_slave->state = NoError;
                    modbus_slave->length = 0x01;
                    modbus_slave->add_offset = address - pass_reg->holdingReg_start_address;
                    *(pass_reg->holdingReg + modbus_slave->add_offset) = *(rxBuffer+4)<<8|*(rxBuffer+5);
                    modbus_slave->holdingReg_start_address = pass_reg->holdingReg_start_address;
                    modbus_slave->comingReg_start_address = pass_reg->comingReg_start_address;
                    modbus_slave->holdingReg = pass_reg->holdingReg;
                    modbus_slave->comingReg = pass_reg->comingReg;
                    break;
                }
            }
            if(pass_reg==NULL)
                modbus_slave->state = AddError;
            break;
        case 0x03:  //读取保持寄存器
            address = *(rxBuffer+2)<<8|*(rxBuffer+3);
            modbus_slave->length = *(rxBuffer+4)<<8|*(rxBuffer+5);
            modbus_slave->function_code = 0x03;
            for(pass_reg=Head_Reg; pass_reg!=NULL; pass_reg=pass_reg->Next)
            {
                if((address>=pass_reg->holdingReg_start_address)&&(address<=pass_reg->holdingReg_end_address))
                {
                    if((address+modbus_slave->length-1)>pass_reg->holdingReg_end_address)
                    {
                        modbus_slave->state = LenError;
                    }else {
                        modbus_slave->state = NoError;
                        modbus_slave->add_offset = address - pass_reg->holdingReg_start_address;
                        modbus_slave->holdingReg_start_address = pass_reg->holdingReg_start_address;
                        modbus_slave->comingReg_start_address = pass_reg->comingReg_start_address;
                        modbus_slave->holdingReg = pass_reg->holdingReg;
                        modbus_slave->comingReg = pass_reg->comingReg;
                    }
                    break;
                }
            }
            if(pass_reg==NULL)
                modbus_slave->state = AddError;
            break;
        case 0x04:  //读取输入寄存器
            address = *(rxBuffer+2)<<8|*(rxBuffer+3);
            modbus_slave->length = *(rxBuffer+4)<<8|*(rxBuffer+5);
            modbus_slave->function_code = 0x04;
            for(pass_reg=Head_Reg; pass_reg!=NULL; pass_reg=pass_reg->Next)
            {
                if((address>=pass_reg->comingReg_start_address)&&(address<=pass_reg->comingReg_end_address))
                {
                    if((address+modbus_slave->length-1)>pass_reg->comingReg_end_address)
                    {
                      modbus_slave->state = LenError;
                    }else{
                      modbus_slave->state = NoError;
                      modbus_slave->add_offset = address - pass_reg->comingReg_start_address;
                      modbus_slave->holdingReg_start_address = pass_reg->holdingReg_start_address;
                      modbus_slave->comingReg_start_address = pass_reg->comingReg_start_address;
                      modbus_slave->holdingReg = pass_reg->holdingReg;
                      modbus_slave->comingReg = pass_reg->comingReg;
                    }
                    break;
                }
            }
            if(pass_reg==NULL)
                modbus_slave->state = AddError;
            break;
        case 0x10:  //写多个保持寄存器
            address = *(rxBuffer+2)<<8|*(rxBuffer+3);
            modbus_slave->length = *(rxBuffer+4)<<8|*(rxBuffer+5);
            modbus_slave->function_code = 0x10;
             if((modbus_slave->length*2!=*(rxBuffer+6))||(*(rxBuffer+6)!=(length-9)))
            {
                modbus_slave->state = LenError;
                break;
            }else {
                for(pass_reg=Head_Reg; pass_reg!=NULL; pass_reg=pass_reg->Next)
                {
                    if((address>=pass_reg->holdingReg_start_address)&&(address<=pass_reg->holdingReg_end_address))
                    {
                        if((address+modbus_slave->length-1)>pass_reg->holdingReg_end_address)
                        {
                            modbus_slave->state = LenError;
                        }else {
                            modbus_slave->state = NoError;
                            modbus_slave->add_offset = address - pass_reg->holdingReg_start_address;
                            for( i=0; i<modbus_slave->length; i++)
                             {
                                *(pass_reg->holdingReg + modbus_slave->add_offset+i) = *(rxBuffer+7+2*i)<<8|*(rxBuffer+8+2*i);
                             }
                            modbus_slave->holdingReg_start_address = pass_reg->holdingReg_start_address;
                            modbus_slave->comingReg_start_address = pass_reg->comingReg_start_address;
                            modbus_slave->holdingReg = pass_reg->holdingReg;
                            modbus_slave->comingReg = pass_reg->comingReg;
                        }
                        break;
                    }
                }
                if(pass_reg==NULL)
                    modbus_slave->state = AddError;
            }
            break;
        default:
            modbus_slave->state = FunCodeError;
            break;
       }
    }else {
        modbus_slave->state = NoACK;
    }
}

uint16_t ModbusSlave_Send_DataProcess(ModbusSlave_Str *modbus_slave,uint8_t *txBuffer)
{
    uint16_t crc;
    uint8_t i=0;
    switch(modbus_slave->state)
    {
    case AddError:
        *txBuffer = modbus_slave->slave_address;
        *(txBuffer+1) = 0x80 + modbus_slave->function_code;
        *(txBuffer+2) = 0x02;
        crc = modbus_crc16(txBuffer,3);
        *(txBuffer+3) = (uint8_t)(crc>>8);
        *(txBuffer+4) = (uint8_t)(crc&0xff);
        return 5;
    case LenError:
        *txBuffer = modbus_slave->slave_address;
        *(txBuffer+1) = 0x80 + modbus_slave->function_code;
        *(txBuffer+2) = 0x03;
        crc = modbus_crc16(txBuffer,3);
        *(txBuffer+3) = (uint8_t)(crc>>8);
        *(txBuffer+4) = (uint8_t)(crc&0xff);
        return 5;
        break;
    case FunCodeError:
        *txBuffer = modbus_slave->slave_address;
        *(txBuffer+1) = 0x80 + modbus_slave->function_code;
        *(txBuffer+2) = 0x01;
        crc = modbus_crc16(txBuffer,3);
        *(txBuffer+3) = (uint8_t)(crc>>8);
        *(txBuffer+4) = (uint8_t)(crc&0xff);
        return 5;
    case NoError:
        *txBuffer = modbus_slave->slave_address;
        *(txBuffer+1) = modbus_slave->function_code;
        switch(modbus_slave->function_code)
        {
        case 0x06:  //写单个保持寄存器
            *(txBuffer+2)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)>>0x08;
            *(txBuffer+3)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)&0xff;
            *(txBuffer+4)=*(modbus_slave->holdingReg + modbus_slave->add_offset)>>0x08;
            *(txBuffer+5)=*(modbus_slave->holdingReg + modbus_slave->add_offset)&0xff;
            crc = modbus_crc16(txBuffer,6);
            *(txBuffer+6) = (uint8_t)(crc>>8);
            *(txBuffer+7) = (uint8_t)(crc&0xff);
            return 8;
        case 0x10:  //写多个保持寄存器
            *(txBuffer+2)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)>>0x08;
            *(txBuffer+3)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)&0xff;
            *(txBuffer+4)= modbus_slave->length>>0x08;
            *(txBuffer+5)= modbus_slave->length&0xff;
            crc = modbus_crc16(txBuffer,6);
            *(txBuffer+6) = (uint8_t)(crc>>8);
            *(txBuffer+7) = (uint8_t)(crc&0xff);
            return 8;
        case 0x03:  //读取保持寄存器
            *(txBuffer+2)= modbus_slave->length*2;
            for( i=0; i<modbus_slave->length; i++)
            {
                *(txBuffer+3+2*i)=*(modbus_slave->holdingReg + modbus_slave->add_offset+i)>>0x08;
                *(txBuffer+4+2*i)=*(modbus_slave->holdingReg + modbus_slave->add_offset+i)&0xff;
            }
            crc = modbus_crc16(txBuffer,3+2*i);
            *(txBuffer+3+2*i) = (uint8_t)(crc>>8);
            *(txBuffer+4+2*i) = (uint8_t)(crc&0xff);
            return (5+2*i);
        case 0x04:  //读取输入寄存器
            *(txBuffer+2)= modbus_slave->length*2;
            for( i=0; i<modbus_slave->length; i++)
            {
                *(txBuffer+3+2*i)=*(modbus_slave->comingReg + modbus_slave->add_offset+i)>>0x08;
                *(txBuffer+4+2*i)=*(modbus_slave->comingReg + modbus_slave->add_offset+i)&0xff;
            }
            crc = modbus_crc16(txBuffer,3+2*i);
            *(txBuffer+3+2*i) = (uint8_t)(crc>>8);
            *(txBuffer+4+2*i) = (uint8_t)(crc&0xff);
            return (5+2*i);
        default:
            return 0;
        }
        break;
      default:
        return 0;
    }
}


