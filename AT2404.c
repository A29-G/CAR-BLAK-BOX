#include <xc.h>
#include "i2c.h"
#include "AT24C04.h"

void write_AT24C04(unsigned char address, unsigned char data)
{
	i2c_start();  //SEN=1
	i2c_write(SLAVE_WRITE);  //1101000 slave id  0 data address
	i2c_write(address);
	i2c_write(data);
	i2c_stop(); //PEN bit 1
    for(unsigned int i=0;i<2000;i++);
}

unsigned char read_AT24C04(unsigned char address)
{
	unsigned char data;

	i2c_start(); //SEN=1  
	i2c_write(SLAVE_WRITE); //1101000 slave id  0 data address
	i2c_write(address);
	i2c_rep_start(); // RSEN = 1 same slave 
	i2c_write(SLAVE_READ); //0x   D  slave id 1 data address
	data = i2c_read();
	i2c_stop();  //PEN bit 1

	return data;
}