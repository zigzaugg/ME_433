#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include "i2c.h"

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


#define EXPADR 0b0100111

void init_expander(void);
void setExpander(char pin, char level);
char getExpander(void);

int main() {
    
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    init_expander();
    __builtin_enable_interrupts();

    char r;
    while(1){
        r = getExpander();
        
        if ((r>>7)&1){
            setExpander(0, 1);
        } else {
            setExpander(0, 0);
        }
      
    }
}

void init_expander(){
    
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    
    i2c_master_setup();   
    
    i2c_master_start();
    i2c_master_send(EXPADR<<1|0); // write the address, or'ed with a 0 to indicate writing
    i2c_master_send(0x00); // the register to write to
    i2c_master_send(0b11110000); // the value to put in the register
    i2c_master_stop();
    
    i2c_master_start();
    i2c_master_send(EXPADR<<1|0); // write the address, or'ed with a 0 to indicate writing
    i2c_master_send(0x06); // the register to write to
    i2c_master_send(0b11110000); // the value to put in the register
    i2c_master_stop();
}

void setExpander(char pin, char level){ //NOT MODULAR
    char newval = level<<pin;
    
    /*if (level){
        newval = old | 1<<pin;
    } else {
        newval = old & (0b11111111 ^ (1<<pin));
    }*/
    
    i2c_master_start();
    i2c_master_send(EXPADR<<1|0); // write the address, or'ed with a 0 to indicate writing
    i2c_master_send(0x09); // the register to write to
    i2c_master_send(newval); // the value to put in the register
    i2c_master_stop();
}

char getExpander(){
    i2c_master_start(); // make the start bit
    i2c_master_send(EXPADR<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(0x09); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(EXPADR<<1|1); // write the address, shifted left by 1, or'ed with a 1 to indicate reading
    char r = i2c_master_recv(); // save the value returned
    i2c_master_ack(1); // make the ack so the slave knows we got it
    i2c_master_stop(); // make the stop bit
    
    return r;
}