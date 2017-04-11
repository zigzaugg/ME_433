#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <math.h>

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


#define CS LATAbits.LATA0

void setVoltage(char channel, char voltage);
unsigned char spi_io(unsigned char o);
void spi_init();
unsigned char Sinfunc(int ii);
unsigned char Sawfunc(int ii);


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

    __builtin_enable_interrupts();

    int ii = 0;
    while(1) {
        _CP0_SET_COUNT(0);
        ii++;
        if(ii > 200){
            ii= 0;
        }
        while(_CP0_GET_COUNT() < 24000) {
            setVoltage(0, Sinfunc(ii));
            setVoltage(1, Sawfunc(ii));
        }
        
    }
}

unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

void spi_init(){
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    
    RPA0Rbits.RPA0R = 0b0011; //chip select
    RPA1Rbits.RPA1R = 0b0011; //SDO
    
    CS = 1;
    
    // setup spi1 ????TODO
    SPI1CON = 0;              // turn off the spi module and reset it
    SPI1BUF;                  // clear the rx buffer by reading from it
    SPI1BRG = 0x3;            // baud rate to 10 MHz [SPI4BRG = (80000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0;  // clear the overflow bit
    SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1;    // master operation
    SPI1CONbits.ON = 1;       // turn on spi 4
      
}

void setVoltage(char channel, char voltage){
    
    unsigned char one, two;
    one = 0b00110000;
    two = 0b00000000;
    
    one += channel<<7;//set channel
    one += voltage>>4; //set first half of voltage
    two = voltage<<4; //set second half of voltage
    
    CS = 0;
    spi_io(one);
    spi_io(two);
    CS = 1;
}

unsigned char Sinfunc(int ii){
    return (unsigned char)(255*sin(3.14159*((double)ii)/50));
}
unsigned char Sawfunc(int ii){
    return (unsigned char)(255.0*ii/200.0);
}