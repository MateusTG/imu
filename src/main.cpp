#define __AVR_ATmega328__

#include "mpu6050.h"
#include "mpu6050.cpp"
#include "esp.cpp"

void USART_Init()
{
  
  //Para Fosc = 16M, U2Xn = 0 e um baud rate = 9600, temos que UBRR0 = 103d = 00000000 01100111
  //Para um baud rate = 115200, temos UBRR0 = 8d = 00000000 00001000
  UBRR0H = 0b00000000;//115200 bits/s.
  UBRR0L = 0b00001000;

  UCSR0A = 0b01000000; //TXC0 = 1 (zera esse flag), U2X0 = 0 (velocidade normal),
  					 //MPCM0 = 0 (desabilita modo multiprocessador).
  UCSR0B = 0b00011000; //Desabilita interrupção recepção (RXCIE0 = 0), transmissão (TXCIE0 = 0)
  					 //e buffer vazio (UDRIE0=0), habilita receptor USART RXEN0 = 1
  					 //(RxD torna-se uma entrada), habilita transmissor TXEN0 = 1
  					 //(TxD torna-se uma saída), seleciona 8 bits de dados (UCSZ2 = 0).
  UCSR0C = 0b00000110; //Habilita o modo assíncrono (UMSEL01/00 = 00), desabilita paridade (UPM01/00 = 00),
  					 //Seleciona um bit de stop (USBS0 = 0), seleciona 8 bits de dados (UCSZ1/0 = 11) e
  					 //sem polaridade (UCPOL0 = 0 - modo assíncrono).
}

char USART_getByte()
{
    while((UCSR0A & 0x80) != 0x80){}; //Espera a recepção de dados.
                                      //RXC0 é zerado após a leitura de UDR0
    return UDR0;//Armazena os dados recebidos em data.
}

void USART_readString(char *str)
{
    int n=0;
    str[n] = '\0';
    do
    {
        str[n] = USART_getByte();
    }
    while (str[n++] != '\n');
}

void USART_sendByte(unsigned char data)
{
	while((UCSR0A & 0x20) != 0x20){}//Verifica se UDR0 pode receber novos dados para Tx.									//
	UDR0 = data;
	while((UCSR0A & 0x40) != 0x40){}//Verifica se os dados já foram "transmitidos".
	UCSR0A = 0x40;//Para zerar TXC0 (bit 6).

}

void USART_writeString(const char *str)
{
	while (*str != '\0')
	{
		USART_sendByte(*str);
		++str;
	}
}


int main(void)
{
  char _buffer[150];
  uint8_t Connect_Status;
  #ifdef SEND_DEMO
  uint8_t Sample = 0;
  #endif

  long int status,i;
  long int AccelX,AccelY,AccelZ, Temperatura, gyroX,gyroY,gyroZ;
  char Data[30];

  USART_Init();
  USART_writeString("Started!\r\n");
  USART_writeString("USART OK!\r\n");

  while(!ESP8266_Begin());
  ESP8266_WIFIMode(BOTH_STATION_AND_ACCESPOINT);/* 3 = Both (AP and STA) */
  ESP8266_ConnectionMode(SINGLE);		/* 0 = Single; 1 = Multi */
  ESP8266_ApplicationMode(NORMAL);	/* 0 = Normal Mode; 1 = Transperant Mode */  

  if(ESP8266_connected() == ESP8266_NOT_CONNECTED_TO_AP)
  ESP8266_JoinAccessPoint(SSID, PASSWORD);
  ESP8266_Start(0, DOMAIN, PORT);
  while(1){
	  Connect_Status = ESP8266_connected();
	  if(Connect_Status == ESP8266_NOT_CONNECTED_TO_AP)
	  ESP8266_JoinAccessPoint(SSID, PASSWORD);
	  if(Connect_Status == ESP8266_TRANSMISSION_DISCONNECTED)
	  ESP8266_Start(0, DOMAIN, PORT);

	  #ifdef SEND_DEMO
	  memset(_buffer, 0, 150);
	  sprintf(_buffer, "GET /update?api_key=%s&field1=%d", API_WRITE_KEY, Sample++);
	  ESP8266_Send(_buffer);
	  _delay_ms(15000);	/* Thingspeak server delay */
	  #endif
		
	  #ifdef RECEIVE_DEMO
	  memset(_buffer, 0, 150);
	  sprintf(_buffer, "GET /channels/%s/feeds/last.txt", CHANNEL_ID);
	  ESP8266_Send(_buffer);
	  Read_Data(_buffer);
	  _delay_ms(600);
  	  #endif
  }  
  
  Twi_Init();
  USART_writeString("I2C OK!\r\n");
  MPU6050_Init();
  USART_writeString("MPU OK!\r\n");

  USART_writeString("READING!\r\n");
  _delay_ms(5000);
  
  while (1)
  {
    Twi_Start();
    Twi_Write( MPU6050_ADDRESS );
    Twi_Write( MPU6050_RA_ACCEL_XOUT_H );
    _delay_us(20);
    Twi_Start();
    Twi_Write( MPU6050_ADDRESS | 1 );
    AccelX = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
    AccelY = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
    AccelZ = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
    Temperatura = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
    gyroX = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
    gyroY = ( TWI_ReadACK() << 8 ) | TWI_ReadACK();
    gyroZ = ( TWI_ReadACK() << 8 ) | TWIReadNACK();
    Twi_Stop();

    Temperatura = Temperatura + 12421;
    Temperatura = Temperatura / 340;

    gyroX += 697;
    
    USART_sendByte(0xff);

    sprintf(Data, "%06ld,%06ld,%06ld,", AccelX, AccelY, AccelZ);
    USART_writeString(Data);
    sprintf(Data, "%06ld,%06ld,%06ld,", gyroX, gyroY, gyroZ);
    USART_writeString(Data);
    sprintf(Data, "%06ld,\n", Temperatura);
    USART_writeString(Data);

    _delay_ms(1);

  }
}
