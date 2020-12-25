#include <gd32vf103v_rvstar.h>

/*
void I2C_init(void);
void SHT20_Call (void);
float SHT20_Measure_Humidity(void);
float SHT20_Measure_Temperature(void);
float SHT20_Measure(char MeasureTorH);
uint8_t SHT20_CheckSum_CRC8(char* Result);
float SHT20_Calculate(char TorR, uint16_t data);
void SHT20_Reset(void);
void I2C_Delay_Us(uint16_t n);
void I2C_Delay_Ms(uint16_t n);
*/

void SHT20_init(void)
{
    I2C_init();
    I2C_config();
}

/*!
    \brief      configure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
*/
void I2C_init(void)
{
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* connect PB6 to I2C0_SCL */
    /* connect PB7 to I2C0_SDA */
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
}

/*!
    \brief      configure the I2C0 interfaces
    \param[in]  none
    \param[out] none
    \retval     none
*/
void I2C_config(void)
{
    /* enable I2C clock */
    rcu_periph_clock_enable(RCU_I2C0);
    /* configure I2C clock */
    i2c_clock_config(I2C0,100000,I2C_DTCY_2);
    /* configure I2C address */
    i2c_mode_addr_config(I2C0,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,0x40);
    /* enable I2C0 */
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0,I2C_ACK_ENABLE);
}

/**
    \brief      1us delay
    \param[in]  none
    \param[out] none
    \retval     none
*/
void I2C_Delay_Us(uint16_t n)
{
    uint8_t i;

    while(n--) {
        for (i = 0; i < 15; i++) {
            __asm("NOP");
        }
    }
}

/**
    \brief      measure the value of sht20 sensor
    \param[in]  none
    \param[out] none
    \retval     none
*/
float SHT20_Measure(char MeasureTorH) {
    
    uint8_t i;
    char result[3] = { 0 };
    /* wait until I2C bus is idle */
    while(i2c_flag_get(I2C0, I2C_FLAG_I2CBSY));
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, SHT20_ADDRESS_W, I2C_TRANSMITTER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));    
    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C0,I2C_FLAG_ADDSEND);
    /* wait until the transmit data buffer is empty */
    while( SET != i2c_flag_get(I2C0, I2C_FLAG_TBE));
    /* enable I2C0*/
    i2c_enable(I2C0);
    if (MeasureTorH == 'H') {
        /* send the EEPROM's internal address to write to : only one byte address */
        i2c_data_transmit(I2C0, SHT20_N_RH_CMD);
    } else {
        i2c_data_transmit(I2C0, SHT20_N_T_CMD);
    }
    /* wait until BTC bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
    I2C_Delay_Us(20);
    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);
    while(I2C_CTL0(I2C0)&0x0200);
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);    
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));    
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, SHT20_ADDRESS_W, I2C_RECEIVER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
        /* send a stop condition to I2C bus */
        i2c_stop_on_bus(I2C0);
        break;
    }
    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C0,I2C_FLAG_ADDSEND);
    /* wait until the stop condition is finished */
    while(I2C_CTL0(I2C0)&0x0200);
    delay_1ms(70);
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);    
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));    
    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, SHT20_ADDRESS_W, I2C_RECEIVER);
    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));    
    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C0,I2C_FLAG_ADDSEND);
    for( i = 0; i < 3; i++) {
        if(0 == i) {
            /* wait until the second last data byte is received into the shift register */
            while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));
            /* disable acknowledge */
            i2c_ack_config(I2C0, I2C_ACK_DISABLE);
        }
        if(1 == i) {
            /* wait until BTC bit is set */
            while(!i2c_flag_get(I2C0, I2C_FLAG_BTC));            
            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2C0);
        }
        if(i2c_flag_get(I2C0, I2C_FLAG_RBNE)) {
            /* read a byte from the EEPROM */
            result[i] = i2c_data_receive(I2C0);            
        } 
    }
    /* wait until the stop condition is finished */
    while(I2C_CTL0(I2C0)&0x0200);    
    /* enable acknowledge */
    i2c_ack_config(I2C0,I2C_ACK_ENABLE);
    i2c_ackpos_config(I2C0,I2C_ACKPOS_CURRENT);
    if (!SHT20_CheckSum_CRC8(result)) {
        /* check CRC8 */
        printf("%s","check fail");
    }
    else if (MeasureTorH == 'H') {
        return SHT20_Calculate('H', (result[0] << 8) + result[1]);
    } else {
        return SHT20_Calculate('T', (result[0] << 8) + result[1]);
    }
}

/**
    \brief      check the sum of CRC8
    \param[in]  none
    \param[out] none
    \retval     none
*/
uint8_t SHT20_CheckSum_CRC8(char* Data) 
{
    char data[2];
    data[0] = Data[0];
    data[1] = Data[1];

    uint32_t POLYNOMIAL = 0x131;
    char crc = 0;
    char bit = 0;
    char byteCtr = 0;
    /* calculates 8-Bit checksum with given polynomial */
    for (byteCtr = 0; byteCtr < 2; ++byteCtr) {
        crc ^= (data[byteCtr]);
        for (bit = 8; bit > 0; --bit) {
            if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else crc = (crc << 1);
        }
    }
    if (crc == Data[2]) {
        return 1;
    }
    else {
        return 0;
    }
}

/**
    \brief      calculate the value of sht20 sensor
    \param[in]  none
    \param[out] none
    \retval     none
*/
float SHT20_Calculate(char TorR, uint16_t data) 
{
    data &= 0xfffc;
    if (TorR == 'H') {
        return (data *125.0 / 65536.0) - 6;
    } 
    else {
        return (data *175.72 / 65536.0) - 46.85;
    }
}