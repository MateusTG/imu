#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include "mpu6050.h"

// INITIAL CONFIG
void Twi_Init(void)
{
    //set SCL to 400kHz
    TWSR = 0x00;
    TWBR = 0x0C;
    //enable TWI
    TWCR = (1<<TWEN);
}

//SEND START SIGNAL
void Twi_Start(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

//SEND STOP SIGNAL
void Twi_Stop(void)
{
    //TWCR = 0xC5;
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

//SEND WRITE SIGNAL
void Twi_Write(uint8_t u8data)
{
    TWDR = u8data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

//SEND READ SIGNAL
uint8_t TWI_ReadACK(void)
{
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

//read byte with NACK
uint8_t TWIReadNACK(void)
{
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

//
uint8_t TWIGetStatus(void)
{
    uint8_t status;
    //mask status
    status = TWSR & 0xF8;
    return status;
}

void MPU6050_Init()
{
  Twi_Start();
  Twi_Write( MPU6050_ADDRESS );
  Twi_Write( MPU6050_RA_PWR_MGMT_1 );
  Twi_Write( 2 ); //Sleep OFF          // 0b 0000 0010
  Twi_Stop();

  _delay_ms(1);

  Twi_Start();
  Twi_Write( MPU6050_ADDRESS );
  Twi_Write( MPU6050_RA_GYRO_CONFIG );
  Twi_Write( 0 );
  Twi_Stop();

  _delay_ms(1);

  Twi_Start();
  Twi_Write( MPU6050_ADDRESS );
  Twi_Write( MPU6050_RA_ACCEL_CONFIG );
  Twi_Write( 0 );
  Twi_Stop();

  Twi_Start();
  Twi_Write( MPU6050_ADDRESS );
  Twi_Write( MPU6050_RA_SMPLRT_DIV );
  Twi_Write( 5 );
  Twi_Stop();

}
