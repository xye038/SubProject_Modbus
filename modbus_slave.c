#include "modbus_crc.h"
#include "modbus_slave.h"

static ModbusSlave_Reg_t *Head_Reg = NULL;
static void Add_ModbusSlave_Reg(ModbusSlave_Reg_t *ModbusSlave_Reg);

void ModbusSlave_Reg_Create(ModbusSlave_Reg_t *ModbusSlave_Reg,
                            uint16_t holdingReg_start_address, uint16_t holdingReg_end_address,
                            uint16_t comingReg_start_address, uint16_t comingReg_end_address,
                            uint16_t* holdingReg, uint16_t* comingReg)
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

void ModbusSlave_Struct_Init(ModbusSlave_Str *SlaveStr, uint8_t ProtoType,
                             uint8_t address, uint16_t Protocol_ID)
{
    SlaveStr->Protocol = ProtoType;
    SlaveStr->slave_address = address;
    SlaveStr->function_code = 0x00;
    SlaveStr->Reg = Head_Reg;
    SlaveStr->length = 0;
    SlaveStr->state = 0;
    SlaveStr->add_offset = 0;
    SlaveStr->UnitID = address;
    if(SlaveStr->Protocol == Modbus_TCP)
    SlaveStr->ProtoID = Protocol_ID;
}


void ModbusSlave_Receive(ModbusSlave_Str *modbus_slave, uint8_t *rxBuffer, uint32_t length)
{
    uint8_t i;
    uint16_t crc;
    uint16_t crc_data;
    uint16_t address;
    ModbusSlave_Reg_t *pass_reg;
    crc_data = (uint16_t)*(rxBuffer+length-1)<<8|*(rxBuffer+length-2);
    crc = modbus_crc16(rxBuffer,length-2);
    if((crc == crc_data)&&(*rxBuffer == modbus_slave->slave_address))
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
            break;
        default:
            modbus_slave->state = FunCodeError;
            break;
       }
    }
}

void ModbusSlave_Send(void(* DMA_UART_Send)(uint8_t*,uint16_t),
                     ModbusSlave_Str *modbus_slave,uint8_t *txBuffer)
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
        *(txBuffer+3) = (uint8_t)(crc&0xff);
        *(txBuffer+4) = (uint8_t)(crc>>8);
        DMA_UART_Send(txBuffer,5);
        break;
    case LenError:
        *txBuffer = modbus_slave->slave_address;
        *(txBuffer+1) = 0x80 + modbus_slave->function_code;
        *(txBuffer+2) = 0x03;
        crc = modbus_crc16(txBuffer,3);
        *(txBuffer+3) = (uint8_t)(crc&0xff);
        *(txBuffer+4) = (uint8_t)(crc>>8);
        DMA_UART_Send(txBuffer,5);
        break;
    case FunCodeError:
        *txBuffer = modbus_slave->slave_address;
        *(txBuffer+1) = 0x80 + modbus_slave->function_code;
        *(txBuffer+2) = 0x01;
        crc = modbus_crc16(txBuffer,3);
        *(txBuffer+3) = (uint8_t)(crc&0xff);
        *(txBuffer+4) = (uint8_t)(crc>>8);
        DMA_UART_Send(txBuffer,5);
        break;
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
            *(txBuffer+6) = (uint8_t)(crc&0xff);
            *(txBuffer+7) = (uint8_t)(crc>>8);
            DMA_UART_Send(txBuffer,8);
            break;
        case 0x10:  //写多个保持寄存器
            *(txBuffer+2)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)>>0x08;
            *(txBuffer+3)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)&0xff;
            *(txBuffer+4)= modbus_slave->length>>0x08;
            *(txBuffer+5)= modbus_slave->length&0xff;
            crc = modbus_crc16(txBuffer,6);
            *(txBuffer+6) = (uint8_t)(crc&0xff);
            *(txBuffer+7) = (uint8_t)(crc>>8);
            DMA_UART_Send(txBuffer,8);
            break;
        case 0x03:  //读取保持寄存器
            *(txBuffer+2)= modbus_slave->length*2;
            for( i=0; i<modbus_slave->length; i++)
            {
                *(txBuffer+3+2*i)=*(modbus_slave->holdingReg + modbus_slave->add_offset+i)>>0x08;
                *(txBuffer+4+2*i)=*(modbus_slave->holdingReg + modbus_slave->add_offset+i)&0xff;
            }
            crc = modbus_crc16(txBuffer,3+2*i);
            *(txBuffer+3+2*i) = (uint8_t)(crc&0xff);
            *(txBuffer+4+2*i) = (uint8_t)(crc>>8);
            DMA_UART_Send(txBuffer,5+2*i);
            break;
        case 0x04:  //读取输入寄存器
            *(txBuffer+2)= modbus_slave->length*2;
            for( i=0; i<modbus_slave->length; i++)
            {
                *(txBuffer+3+2*i)=*(modbus_slave->comingReg + modbus_slave->add_offset+i)>>0x08;
                *(txBuffer+4+2*i)=*(modbus_slave->comingReg + modbus_slave->add_offset+i)&0xff;
            }
            crc = modbus_crc16(txBuffer,3+2*i);
            *(txBuffer+3+2*i) = (uint8_t)(crc&0xff);
            *(txBuffer+4+2*i) = (uint8_t)(crc>>8);
            DMA_UART_Send(txBuffer,5+2*i);
            break;
        default:
            break;
        }
    }
}


void ModbusSlave_TCP_Receive(ModbusSlave_Str *modbus_slave, uint8_t *rxBuffer, uint32_t length)
{
    uint8_t i;
    uint16_t Transaction_ID;
    uint16_t Protocol_ID;
    uint16_t address;
    ModbusSlave_Reg_t *pass_reg;
    /**************事务处理标识符**************/
    Transaction_ID = *rxBuffer<<8|*(rxBuffer+1);
    modbus_slave->TransID = Transaction_ID;
    /**************协议标识符****************/
    Protocol_ID = *(rxBuffer+2)<<8|*(rxBuffer+3);
    /***************数据长度*****************/
    modbus_slave->DataLength = *(rxBuffer+4)<<8|*(rxBuffer+5);
    if((*(rxBuffer+6) == modbus_slave->UnitID)&&(Protocol_ID == modbus_slave->ProtoID)\
            &&((length-8)==modbus_slave->DataLength))
    {
       switch(*(rxBuffer+7))
       {
       case 0x06:   //写单个保持寄存器
           address = *(rxBuffer+8)<<8|*(rxBuffer+9);
           modbus_slave->function_code = 0x06;
           for(pass_reg=Head_Reg; pass_reg!=NULL; pass_reg=pass_reg->Next)
           {
               if((address>=pass_reg->holdingReg_start_address)&&(address<=pass_reg->holdingReg_end_address))
               {
                   modbus_slave->state = NoError;
                   modbus_slave->length = 0x01;
                   modbus_slave->add_offset = address - pass_reg->holdingReg_start_address;
                   *(pass_reg->holdingReg + modbus_slave->add_offset) = *(rxBuffer+10)<<8|*(rxBuffer+11);
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
           address = *(rxBuffer+8)<<8|*(rxBuffer+9);
           modbus_slave->length = *(rxBuffer+10)<<8|*(rxBuffer+11);
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
           address = *(rxBuffer+8)<<8|*(rxBuffer+9);
           modbus_slave->length = *(rxBuffer+10)<<8|*(rxBuffer+11);
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
           address = *(rxBuffer+8)<<8|*(rxBuffer+9);
           modbus_slave->length = *(rxBuffer+10)<<8|*(rxBuffer+11);
           modbus_slave->function_code = 0x10;
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
                           *(pass_reg->holdingReg + modbus_slave->add_offset+i) = *(rxBuffer+13+2*i)<<8|*(rxBuffer+14+2*i);
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
           break;
        default:
            modbus_slave->state = FunCodeError;
            break;
       }
    }
    else {
        modbus_slave->state = MBAPError;
    }
}


uint8_t ModbusSlave_TCP_Send(uint8_t(* DMA_UART_Send)(uint8_t*,uint16_t),
                     ModbusSlave_Str *modbus_slave,uint8_t *txBuffer)
{
    uint8_t i=0;

    *txBuffer = modbus_slave->TransID>>8;
    *(txBuffer+1) = modbus_slave->TransID&0xFF;
    *(txBuffer+2) = modbus_slave->ProtoID>>8;
    *(txBuffer+3) = modbus_slave->ProtoID&0xFF;
    switch(modbus_slave->state)
    {
    case AddError:
        *(txBuffer+4) = 0x00;
        *(txBuffer+5) = 0x03;
        *(txBuffer+6) = modbus_slave->UnitID;
        *(txBuffer+7) = 0x80 + modbus_slave->function_code;
        *(txBuffer+8) = 0x02;
        return DMA_UART_Send(txBuffer,9);
        break;
    case LenError:
        *(txBuffer+4) = 0x00;
        *(txBuffer+5) = 0x03;
        *(txBuffer+6) = modbus_slave->UnitID;
        *(txBuffer+7) = 0x80 + modbus_slave->function_code;
        *(txBuffer+8) = 0x03;
        return DMA_UART_Send(txBuffer,9);
        break;
    case FunCodeError:
        *(txBuffer+4) = 0x00;
        *(txBuffer+5) = 0x03;
        *(txBuffer+6) = modbus_slave->UnitID;
        *(txBuffer+7) = 0x80 + modbus_slave->function_code;
        *(txBuffer+8) = 0x01;
        return DMA_UART_Send(txBuffer,9);
        break;
    case MBAPError:
        *(txBuffer+4) = 0x00;
        *(txBuffer+5) = 0x03;
        *(txBuffer+6) = modbus_slave->UnitID;
        *(txBuffer+7) = 0x80 + modbus_slave->function_code;
        *(txBuffer+8) = 0x0A;
        return DMA_UART_Send(txBuffer,9);
        break;
    case NoError:
        switch(modbus_slave->function_code)
        {
        case 0x06:  //写单个保持寄存器
            *(txBuffer+4) = 0x00;
            *(txBuffer+5) = 0x06;
            *(txBuffer+6) = modbus_slave->UnitID;
            *(txBuffer+7) = modbus_slave->function_code;
            *(txBuffer+8)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)>>0x08;
            *(txBuffer+9)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)&0xff;
            *(txBuffer+10)=*(modbus_slave->holdingReg + modbus_slave->add_offset)>>0x08;
            *(txBuffer+11)=*(modbus_slave->holdingReg + modbus_slave->add_offset)&0xff;
            return DMA_UART_Send(txBuffer,12);
            break;
        case 0x10:  //写多个保持寄存器
            *(txBuffer+4) = 0x00;
            *(txBuffer+5) = 0x06;
            *(txBuffer+6) = modbus_slave->UnitID;
            *(txBuffer+7) = modbus_slave->function_code;
            *(txBuffer+8)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)>>0x08;
            *(txBuffer+9)=(modbus_slave->holdingReg_start_address + modbus_slave->add_offset)&0xff;
            *(txBuffer+10)= modbus_slave->length>>0x08;
            *(txBuffer+11)= modbus_slave->length&0xff;
            return DMA_UART_Send(txBuffer,12);
            break;
        case 0x03:  //读取保持寄存器
            *(txBuffer+4) = 0x00;
            *(txBuffer+5) = modbus_slave->length*2+3;
            *(txBuffer+6) = modbus_slave->UnitID;
            *(txBuffer+7) = modbus_slave->function_code;
            *(txBuffer+8)= modbus_slave->length*2;
            for( i=0; i<modbus_slave->length; i++)
            {
                *(txBuffer+9+2*i)=*(modbus_slave->holdingReg + modbus_slave->add_offset+i)>>0x08;
                *(txBuffer+10+2*i)=*(modbus_slave->holdingReg + modbus_slave->add_offset+i)&0xff;
            }
            return DMA_UART_Send(txBuffer,9+2*i);
            break;
        case 0x04:  //读取输入寄存器
            *(txBuffer+4) = 0x00;
            *(txBuffer+5) = modbus_slave->length*2+3;
            *(txBuffer+6) = modbus_slave->UnitID;
            *(txBuffer+7) = modbus_slave->function_code;
            *(txBuffer+8)= modbus_slave->length*2;
            for( i=0; i<modbus_slave->length; i++)
            {
                *(txBuffer+9+2*i)=*(modbus_slave->comingReg + modbus_slave->add_offset+i)>>0x08;
                *(txBuffer+10+2*i)=*(modbus_slave->comingReg + modbus_slave->add_offset+i)&0xff;
            }
            return DMA_UART_Send(txBuffer,9+2*i);
            break;
        default:
            return 0;
            break;
        }
    }
    return 0;
}

