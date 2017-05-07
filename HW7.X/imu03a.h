#ifndef IMU03A_H__
#define IMU03A_H__

#define IMUADR 0b1101011

void init_gyro(void);
void getData(char * data);
char getValue(char r);

#endif