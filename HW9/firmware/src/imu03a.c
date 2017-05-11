#include <xc.h>
#include "imu03a.h"
#include "ili9163c.h"

void init_gyro(){ 
    i2c_master_start();
    i2c_master_send(IMUADR<<1|0); // write the address, or'ed with a 0 to indicate writing
    i2c_master_send(0x10); // CRTL1_XL
    i2c_master_send(0b10000010);
    i2c_master_stop();
    
    i2c_master_start();
    i2c_master_send(IMUADR<<1|0); // write the address, or'ed with a 0 to indicate writing
    i2c_master_send(0x11); // CRTL1_XL
    i2c_master_send(0b10001000);
    i2c_master_stop();
    
}

void getData(unsigned char* data){
    int ii;
    for (ii = 0; ii < 14; ii++){
        data[ii] = getValue(0x20+ii);
    }
}


char getValue(char r){
    i2c_master_start(); // make the start bit
    i2c_master_send(IMUADR<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(r); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(IMUADR<<1|1);
    r = i2c_master_recv();
    i2c_master_ack(1); // make the ack so the slave knows we got it
    i2c_master_stop();
    
    return r;
}