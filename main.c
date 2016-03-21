
#define F_CPU 8000000
#define ADC_ADDR 0x68

#define BAUD 9600UL      // Baudrate
 
// Berechnungen
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.
 
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch! 
#endif

#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/wdt.h>

#include "i2cmaster.h"
#include "MAX7219.h"

void init_adc()
{
    ADMUX = (1<<REFS0) | (1<<ADLAR); //set adc-ref to vcc
    ADMUX |= (1<<MUX3) | (1<<MUX2) | (1<<MUX1); //set mux to internal band gap

    ADCSRA = (1<<ADPS1) | (1<<ADPS0);     // Frequenzvorteiler
    ADCSRA |= (1<<ADEN);                  // ADC aktivieren

    //dummy read 16 times, because of buggy vcc measurement
    for(uint8_t i=0; i != 16; ++i) {
        ADCSRA |= (1<<ADSC);                  // eine ADC-Wandlung 
        while (ADCSRA & (1<<ADSC) )           // auf Abschluss der Konvertierung warten
        {}
        (void) ADCH;
    }
}

uint8_t measure_vcc()
{
    ADCSRA |= (1<<ADSC);            // eine Wandlung "single conversion"
    while (ADCSRA & (1<<ADSC) ) {   // auf Abschluss der Konvertierung warten
    }
    return ADCH;                    // ADC auslesen und zurückgeben 85=3.3V, 55=5V
}

#define CHANNEL_1 0b00000000
#define CHANNEL_2 0b00100000
uint16_t read_ADC_val(uint8_t channel) {
    uint8_t retH, retL, rdy=1;
    i2c_start_wait(0b11010000); //Adress Byte, Config Byte comeing
    i2c_write(0b10001000 | channel);   //Configuration Byte, Channel 0, Continuous Conversion
    while(rdy==1) {
        i2c_rep_start(0b11010001); //Adress Byte, Data Req
        retH = i2c_readAck();
        retL = i2c_readAck();
        rdy = i2c_readNak() >> 7;
    }
    i2c_stop();

    return (retH << 8) + retL;
}
    
void init(){
    //UART init
    //uart_init();
    Load_High();
    _delay_ms(100);
    //Init MAX7219
    SPIMasterInit();
    set_op_mode(MAX_MODE_SLEEP);
    _delay_ms(100);
    set_op_mode(MAX_MODE_NORMAL);
    set_intensity(0x01);
    set_decoder(0xFF);
    set_scan_limit(0x07);
    set_all_digit_data(chars[7]+128);
    _delay_ms(100);
    i2c_init();
    _delay_ms(100);
    i2c_stop();
    _delay_ms(100);
    i2c_stop();
    init_adc();
    _delay_ms(100);
}

int main(){
    init();
    //uart_puts("Hello World!\n\r");
    wdt_enable(WDTO_500MS);
    while(1){
        int16_t val = read_ADC_val(CHANNEL_1);
        int16_t voltage = (double)(val)*0.938606-3.42159;
        int16_t current = read_ADC_val(CHANNEL_2);
        uint8_t digbuf;

        if(voltage/10000 > 0) {
            digbuf = (voltage/10000)%10;
            set_digit_data(0, digbuf);

            digbuf = (voltage/1000)%10;
            set_digit_data(1, digbuf+128);

            digbuf = (voltage/100)%10;
            set_digit_data(2, digbuf);

            digbuf = (voltage/10)%10;
            set_digit_data(3, digbuf);
        }
        else if(voltage > 0) {
            digbuf = (voltage/1000)%10;
            set_digit_data(0, digbuf+128);

            digbuf = (voltage/100)%10;
            set_digit_data(1, digbuf);

            digbuf = (voltage/10)%10;
            set_digit_data(2, digbuf);

            digbuf = (voltage)%10;
            set_digit_data(3, digbuf);
        }
        else {
            set_digit_data(0, 128);
            set_digit_data(1, 0);
            set_digit_data(2, 0);
            set_digit_data(3, 0);
        }
        char strbuf[20];
        itoa(current, strbuf, 10);
        current = (double)(current)*(-0.133193)+4.51849;
        if(current < 0) {
            set_digit_data(4, 128);
            set_digit_data(5, 0);
            set_digit_data(6, 0);
            set_digit_data(7, 0);
        }
        else {
            digbuf = current%10;
            set_digit_data(7, digbuf);
            digbuf = (current/10)%10;
            set_digit_data(6, digbuf);
            digbuf = (current/100)%10;
            set_digit_data(5, digbuf);
            digbuf = (current/1000)%10;
            set_digit_data(4, digbuf+128);
        }
    //25V - 26610 LSB - 0,0009478673
    //20V - 21335 LSB - 0,0009400263
    //15V - 16016 LSB - 0,0009307520
    //10V - 10644 LSB - 0,0009382623
    //5V  -  5315 LSB - 0,0009429514
    //1V  -  1073 LSB 
    //fit (26610, 25), (21335, 20), (16016, 15), (10644, 10), (5315, 5), (1073, 1)
    //
        if(measure_vcc()>=70) {
            set_op_mode(MAX_MODE_SLEEP);
            i2c_stop();
            while(measure_vcc()>=70) {}
            init();
        }
        wdt_reset();
    }
    return 0;
}