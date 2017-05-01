#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "i2c.h"
#include "ili9163c.h"

// DEVCFG0
#pragma config DEBUG = 0b10 // no debugging
#pragma config JTAGEN = 0 // no jtag
#pragma config ICESEL = 0b11 // use PGED1 and PGEC1
#pragma config PWP = 0b1111111 // no write protect
#pragma config BWP = 1 // no boot write protect
#pragma config CP = 1 // no code protect

// DEVCFG1
#pragma config FNOSC = 0b011 // use primary oscillator with pll
#pragma config FSOSCEN = 0 // turn off secondary oscillator
#pragma config IESO = 0 // no switching clocks
#pragma config POSCMOD = 0b10 // high speed crystal mode
#pragma config OSCIOFNC = 1 // free up secondary osc pins
#pragma config FPBDIV = 0b00 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = 0b11 // do not enable clock switch
#pragma config WDTPS = 0b10100 // slowest wdt
#pragma config WINDIS = 1 // no wdt window
#pragma config FWDTEN = 0 // wdt off by default
#pragma config FWDTWINSZ = 0b11 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = 0b001 // divide input clock to be in range 4-5MHz //divided by 2
#pragma config FPLLMUL = 0b111 // multiply clock after FPLLIDIV // multiplied by 24
#pragma config FPLLODIV = 0b001 // divide clock after FPLLMUL to get 48MHz //divided by 2
#pragma config UPLLIDIV = 0b001 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB?????
#pragma config UPLLEN = 0 // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = 0 // allow multiple reconfigurations
#pragma config IOL1WAY = 0 // allow multiple reconfigurations
#pragma config FUSBIDIO = 1 // USB pins controlled by USB module
#pragma config FVBUSONIO = 1 // USB BUSON controlled by USB module

#define LED LATAbits.LATA4 
#define PUSH_BUTTON PORTBbits.RB4
#define BCKGND MAGENTA
#define TXT YELLOW
#define SLVADR 0b1101011

void init_gyro(void);
void setExpander(char pin, char level);
char getValue(char r);
void drawChar(unsigned short x0, unsigned short y0, char c, unsigned short color);
void drawString(unsigned short x0, unsigned short y0, char *s, unsigned short color);
void drawBar(unsigned short x0, unsigned short y0, short len, unsigned short color);

int main() {
    TRISAbits.TRISA4=0;
    TRISBbits.TRISB4=1;
    LED = 0;
    
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    SPI1_init();
    LCD_init();
    
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    
    i2c_master_setup();
    
    __builtin_enable_interrupts();

    char buff[20];
    char r;
    LCD_clearScreen(BCKGND);
    
    
    /*r = getValue();
    sprintf(buff, "who: %i", r);
    drawString(45, 45, buff, BLUE);
    LED = 1;*/
    
    while(1){
        
        
    }
}

void init_gyro(){ 
    i2c_master_start();
    i2c_master_send(SLVADR<<1|0); // write the address, or'ed with a 0 to indicate writing
    i2c_master_send(0x10); // CRTL1_XL
    i2c_master_send(0b10000010);
    i2c_master_send(0b10001000);
    i2c_master_stop();
}

char getData(char * data){
    
    i2c_master_start(); // make the start bit
    i2c_master_send(SLVADR<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(0x22); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(SLVADR<<1|1); // write the address, shifted left by 1, or'ed with a 1 to indicate reading
    
    int ii;
    for (ii = 0; ii<12; ii++){
        data[ii] = i2c_master_recv(); // save the value returned
        if(ii<11){
            i2c_master_ack(0);
        }
    }
    i2c_master_ack(1); // make the ack so the slave knows we got it
    i2c_master_stop(); // make the stop bit
    
}

char getValue(char r){
    i2c_master_start(); // make the start bit
    i2c_master_send(SLVADR<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(r); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(SLVADR<<1|1);
    r = i2c_master_recv();
    i2c_master_ack(1); // make the ack so the slave knows we got it
    i2c_master_stop();
}

void drawChar(unsigned short x0, unsigned short y0, char c, unsigned short color){
    int ii, jj;
    for(ii = 0; ii<5; ii++){
        for(jj = 0; jj<8; jj++){
            int x = x0+ii;
            int y = y0+jj;
            if (x<128 && y<128){
                if((ASCII[c-0x20][ii]>>jj)&1){
                    LCD_drawPixel(x, y, color);
                } else {
                    LCD_drawPixel(x, y, BCKGND);
                }
            }
        }
    }
}

void drawString(unsigned short x0, unsigned short y0, char *s, unsigned short color){
    int ii=0;
    while(s[ii]){
        char c = s[ii];
        drawChar(x0+ii*6, y0, c, color);
        ii++;
    }
}

void drawBar(unsigned short x0, unsigned short y0, short len, unsigned short color){
    int ii, jj;
    for (ii= 1; ii<128; ii++){
        for(jj = y0; jj<(y0+8); jj++){
            if(((len>0)&&(ii<x0+len)&&(ii>x0))||(len<0)&&(ii<x0)&&(ii>x0+len)){
                LCD_drawPixel(ii, jj, color);
            }else{
                LCD_drawPixel(ii, jj, BCKGND);
            }
        }
    }
}