#include <avr/io.h>

#ifndef MAX7219
#define MAX7219

#define MAX_MODE_SLEEP 0x00
#define MAX_MODE_NORMAL 0x01

//Max7129 Zeichentabelle
extern const uint8_t chars[];

void transmit(unsigned char addr, char data);
void Load_Low(void);
void Load_High(void);

void SPIMasterInit(void);
void set_op_mode(unsigned mode);
void set_decoder(uint8_t mode);
void set_scan_limit(uint8_t mode);
void set_intensity(uint8_t mode);
void DisplayTest(unsigned mode);
void set_digit_data(uint8_t digit, uint8_t data);
void set_all_digit_data(uint8_t data);
void set_digit_num(uint8_t digit, uint8_t num);
void set_all_digit_num(uint8_t num);
void Show2DigitsDots(uint8_t number, uint8_t start_digit, uint8_t dots);

#endif
