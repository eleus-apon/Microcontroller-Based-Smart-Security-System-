/*
 * GccApplication5.c
 *
 * Created: 11/5/2014 7:27:24 PM
 *  Author: Eleus_Apon
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"

void init_int0()
{
	sei();
	MCUCR|=(1<<ISC01)|(1<<ISC00);
	GICR|=(1<<INT0);
}

ISR(INT0_vect)
{
	PORTB&=~(1<<5);
	PORTB&=~(1<<6);
	PORTB&=~(1<<7);
	LCDClear();
	LCDWriteStringXY(0,0,"System Activated");
}

void InitADC()
{
	ADMUX=(1<<REFS0);
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS0); //Div Factor=64
}

uint16_t ReadADC(uint8_t ch)
{
	ch=ch&0b00000111;
	ADMUX|=ch;
	ADCSRA|=(1<<ADSC);
	while(!(ADCSRA & (1<<ADIF)));
	ADCSRA|=(1<<ADIF);
	return(ADC);
}

void USART_Init(void)
{
	UBRRH = 0;
	UBRRL = 25;
	UCSRB = (1<<RXEN) | (1<<TXEN);
	UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);
}

char UReadChar()
{
	while(!(UCSRA & (1<<RXC))); // wait until data has been received
	return UDR; // return the byte
}

void UWriteChar(char d)
{
	while(!(UCSRA & (1<<UDRE))); // wait till UDR is ready
	UDR=d; // send data
}

void UWriteString(const char *cmd)
{
	while((*cmd)!='\0')
	{
		UWriteChar(*cmd);
		cmd++;
	}
}

void Waiting(int j) // simple delay function
{
	uint8_t i;
	for(i=0;i<j;i++)
	_delay_ms(200);
}

void GSM_Notify(char *msg)
{
	char cmd[25];
	sprintf(cmd,"AT+CMGS= +8801791406040");
	cmd[8]=0x22; // " or <34>
	uint8_t n=strlen(cmd);
	cmd[n]=0x22; // " or <34>
	cmd[n+1]='\0';
	UWriteString(cmd);
	UWriteChar(0x0D); //<CR> or <13>
	Waiting(5);
	UWriteString(msg);
	UWriteChar(0x1A);  // <CTRL+Z> or <26>
	Waiting(5);
}

int main()
{
	DDRB&=~(1<<0);
	DDRB|=0b11100000;
	PORTB|=(1<<0);
	PORTB&=~(1<<5);
	PORTB&=~(1<<6);
	PORTB&=~(1<<7);
	uint16_t adc_result1;
	uint16_t adc_result2;
	init_int0();
	InitADC();
	USART_Init(); // initialization of USART
	LCDInit(LS_NONE);
	Waiting(2);
	UWriteString("AT");
	UWriteChar(0x0D); //<CR> or <13>
	Waiting(2);
	LCDWriteStringXY(0,0,"System Activated");
	
    while(1)
    {
		adc_result1=ReadADC(0);
		//adc_result2=ReadADC(1);
		
		if(adc_result1>100)
		{
			LCDClear();
			LCDWriteStringXY(1,0,"Fire Detected");
			PORTB|=(1<<5);
			PORTB|=(1<<6);
			PORTB|=(1<<7);
			GSM_Notify("Fire is detected on your system");
			LCDWriteStringXY(2,1,"Message Sent");
			Waiting(200);
		}
		
		/*if(adc_result2>300)
		{
			LCDClear();
			LCDWriteStringXY(1,0,"Gas Leakage");
			PORTB|=(1<<5);
			PORTB|=(1<<6);
			PORTB|=(1<<7);
			GSM_Notify("Gas leakage is detected on your system");
			LCDWriteStringXY(2,1,"Message Sent");
			Waiting(200);
		}*/
		
		if(bit_is_set(PINB,0))
		{
			LCDClear();
			LCDWriteStringXY(0,0,"Motion Detected");
			PORTB|=(1<<5);
			PORTB|=(1<<6);
			GSM_Notify("A motion is detected on your system");
			LCDWriteStringXY(2,1,"Message Sent");
			Waiting(100);
		}
    }
	return 0;
}