#include "MAX7219.h"

//Max7129 Zeichentabelle
const uint8_t chars[] = {126,48,109,121,51,91,95,112,127,123};

uint8_t  decoder = 0x00;

void Load_Low(void)
{
  PORTB &= ~(1 << PB2);
}

void Load_High(void)
{
  PORTB |= (1 << PB2);
}

void SPIMasterInit(void)
{
    Load_Low();
    DDRB = (1<<PB2)|(1<<PB3)|(1<<PB5);      // set PB2(SS), PB3 (MOSI) and PB5 (SCK) output, all others input
    SPCR = (1<<SPE)|(1<<MSTR);           // enable SPI, Master, set clock rate fck/4
    PORTB |= (1 << PB7)|(1 << PB2);
}

void transmit(unsigned char addr, char data) 
{
    Load_Low();
    SPDR = addr;                      // Start transmission
    while(!(SPSR & (1<<SPIF)));       // Wait for transmission complete

    asm volatile("nop");

    SPDR = data;                      // Start transmission
    while(!(SPSR & (1<<SPIF)));       // Wait for transmission complete
    Load_High();
}

//1 Normal Operation, 0 Shutdown
void set_op_mode(unsigned mode)
{
    transmit(0x0C,mode); 
}

//Bits for digits
void set_decoder(uint8_t mode)
{
    transmit(0x09,mode);    //0x00 no decode / 0xFF all decode
    decoder = 0xFF;
}

//Digits activated. 0x00 to 0x07
void set_scan_limit(uint8_t mode)
{
    transmit(0x0B,mode);
}

// 0 < mode < 0x0F
void set_intensity(uint8_t mode)
{
    transmit(0x0A,mode);       
}

void DisplayTest(unsigned mode)
{
    transmit(0x0F,mode); 
}

uint8_t code[] = {0,1,2,3,4,5,6,7};
//uint8_t code[] = {3,1,2,4,0,5,7,6};

void show_err(void)
{
	if(decoder == 0x00) {
		set_all_digit_data(0);
		transmit(code[0]+1,79);
		transmit(code[1]+1,5);
		transmit(code[2]+1,5); 
	}
	else {
		set_all_digit_data(0x0F);
		transmit(code[0]+1,0x0C);
		transmit(code[1]+1,0x0B);
		transmit(code[2]+1,0x0D);
		transmit(code[3]+1,0x0E);
	}
	while(1);
}

void set_digit_data(uint8_t digit, uint8_t data)
{
	if(digit > 7)
		show_err();
	else
		transmit(code[digit]+1,data);
}

void set_all_digit_data(uint8_t data)
{
	for(uint8_t i = 1; i!=9;i++)
		transmit(i,data);   
}

void set_digit_num(uint8_t digit, uint8_t num)
{
	if(num > 9) show_err();
	else set_digit_data(digit, chars[num]);
}

void set_all_digit_num(uint8_t num)
{
	if(num > 9) show_err();
	else set_all_digit_data(chars[num]);
}

void Show2DigitsDots(uint8_t number, uint8_t start_digit, uint8_t dots)
{
        set_digit_data( start_digit, chars[number / 10] + ((dots & 1) ? 128 : 0 )); 
        set_digit_data( start_digit + 1, chars[number % 10] + ((dots & 2) ? 128 : 0) );  
}

void Show2Digits(uint8_t number, uint8_t start_digit)
{
    Show2DigitsDots(number, start_digit, 0);   
}