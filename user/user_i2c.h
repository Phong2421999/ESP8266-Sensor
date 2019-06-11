#include "osapi.h"
#include "i2c_master.h"
#include "user_interface.h"
#include <stdio.h>

#define slaveAddress 0x44

os_timer_t *timer;

const uint8_t g_polynom = 0x31;

uint8_t ICACHE_FLASH_ATTR  user_data_to_crc8 (uint8_t data[], int len)
{
    // initialization value
    uint8_t crc = 0xff;
    
    // iterate over all bytes
    for (int i=0; i < len; i++)
    {
        crc ^= data[i];  
    
        for (int i = 0; i < 8; i++)
        {
            bool xor = crc & 0x80;
            crc = crc << 1;
            crc = xor ? crc ^ g_polynom : crc;
        }
    }

    return crc;
} 

bool ICACHE_FLASH_ATTR user_check_crc(uint8_t MSB, uint8_t LSB, uint8_t CRC){
    uint8_t data[2];
    
    data[0] = MSB;
    data[1] = LSB;

    uint8_t result = user_data_to_crc8(data,2);

    if(CRC == result){
        os_printf("\r\n Last command successfully");
        return true;
    }
    else
    {
         os_printf("\r\n Last command fail");
        return false;
    }
    
}

void ICACHE_FLASH_ATTR user_i2c_init()
{
    i2c_master_gpio_init();
}
bool ICACHE_FLASH_ATTR user_send_command(uint8_t MSBCommand,uint8_t LSBCommand)
{
        os_printf("\r\n Send MSB command!");
        i2c_master_writeByte(MSBCommand);
        if(i2c_master_checkAck())
        {
            os_printf("\r\n Send MSB command Success!");
            os_printf("\r\n Send LSB command!");
            i2c_master_writeByte(LSBCommand);
            if(i2c_master_checkAck())
            {
                os_printf("\r\n Send LSB command Success!");
                return true;
            }
            else
            {
                os_printf("\r\n Send LSB command fail!");
                i2c_master_stop();
                return false;
            }
            
        }
        else
        {
            os_printf("\r\n Send MSB command fail!");
            i2c_master_stop();
            return false;
        }
}
void ICACHE_FLASH_ATTR user_soft_reset()
{
    i2c_master_start();
    uint8_t startAddress = slaveAddress<<1;
    i2c_master_writeByte(startAddress);
    if(i2c_master_checkAck())
    {
       if(user_send_command(0x30,0xA2))
       {
            os_printf("\r\nSoft reset success!");
            i2c_master_stop();
       }
       else
       {
           os_printf("\r\nSoft reset fail!");
        i2c_master_stop();
       }
       
    }
    else
    {
        os_printf("\r\nSoft reset fail!");
        i2c_master_stop();
    }
    
}

uint8_t ICACHE_FLASH_ATTR user_read_temp()
{
    i2c_master_start();
    uint8_t readAddress = (slaveAddress<<1)+1;      
    i2c_master_writeByte(readAddress);
    if(i2c_master_checkAck())
    {
        uint8_t MSBTempData = i2c_master_readByte();
        i2c_master_send_ack();
        uint8_t LSBTempData = i2c_master_readByte();
        i2c_master_send_ack();
        uint8_t CRCTempData = i2c_master_readByte();
        i2c_master_send_ack();
        uint8_t MSBHuData = i2c_master_readByte();
        i2c_master_send_ack();
        uint8_t LSBHuData = i2c_master_readByte();
        i2c_master_send_ack();
        uint8_t CRCHuData = i2c_master_readByte();
        i2c_master_send_nack();
        os_printf("\r\nRead temp register success!");
        os_printf("\r\nTemp Reigster Result: ");
        os_printf("\r\nMSB: %d",MSBTempData);
        os_printf("\r\nLSB: %d",LSBTempData);
        os_printf("\r\nCRC: %d",CRCTempData);
         os_printf("\r\nRead Hu register success!");
        os_printf("\r\nHu Reigster Result: ");
        os_printf("\r\nMSB: %d",MSBHuData);
        os_printf("\r\nLSB: %d",LSBHuData);
        os_printf("\r\nCRC: %d",CRCHuData);
        i2c_master_stop();
    }
    else
    {
        os_printf("\r\nRead data from sensor fail!");
        i2c_master_stop();
    }
}

uint8_t ICACHE_FLASH_ATTR user_get_data_low()
{
    i2c_master_start();
    uint8_t startAddress = slaveAddress<<1;
    i2c_master_writeByte(startAddress);
    if(i2c_master_checkAck())
    {
        
        if(user_send_command(0x23,0x29))
        {
            os_timer_disarm(&timer);
            os_timer_setfn(&timer,(os_timer_func_t *)user_read_temp,NULL);
            os_timer_arm(&timer,100,0);
        }
        else
        {
            os_printf("\r\nRead data from sensor fail!");
            i2c_master_stop();  
        }
        
    }
    else
    {
        os_printf("\r\nRead data from sensor fail!");
        i2c_master_stop();  
    }
}

void ICACHE_FLASH_ATTR user_read_data_sensor()
{
    i2c_master_start();
    uint8_t startAddress = slaveAddress<<1;
    i2c_master_writeByte(startAddress);
    if(i2c_master_checkAck())
    {
        if(user_send_command(0xF3,0x2D))
        {
            i2c_master_start();
            uint8_t readAddress = (slaveAddress<<1)+1;      
            i2c_master_writeByte(readAddress);
            if(i2c_master_checkAck())
            {
                uint8_t MSBData = i2c_master_readByte();
                i2c_master_send_ack();
                uint8_t LSBData = i2c_master_readByte();
                i2c_master_send_ack();
                uint8_t CRCData = i2c_master_readByte();
                i2c_master_send_nack();
                i2c_master_stop();
                os_printf("\r\nRead status register success!");
                os_printf("\r\nStatus Reigster Result: ");
                os_printf("\r\nMSB: %d",MSBData);
                os_printf("\r\nLSB: %d",LSBData);
                os_printf("\r\nCRC: %d",CRCData);
                if(user_check_crc(MSBData,LSBData,CRCData))
                {
                    user_get_data_low();
                }
                  else
                {
                os_printf("\r\nRead data from sensor fail!");
                i2c_master_stop();
                }
            }
            else
            {
                os_printf("\r\nRead data from sensor fail!");
                i2c_master_stop();
            }
            
        }
        else
        {
            os_printf("\r\nRead data from sensor fail!");
            i2c_master_stop();
        }
    }
    else
    {
        os_printf("\r\nRead data from sensor fail!");
        i2c_master_stop();
    }
    
}
