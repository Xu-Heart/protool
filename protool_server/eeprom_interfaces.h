/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  eeprom_interfaces.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2019年08月16日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月16日 22时01分37秒"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <sys/select.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#ifdef IOCTL_EEPROM

#include<stdio.h>
#include<stdlib.h>
#include<sys/select.h>
#include<sys/time.h>
#include<sys/types.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>

#define SLAVE_ADDR        0x50
#define PAGE_SIZE         0x08
#define I2C_READ_FLAG     0x01
#define I2C_WRITE_FLAG    0x00

//#define __DEBUG
#ifdef __DEBUG
#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) do{} while(0)
#endif

void    msleep(int msec);
int     i2c_read_random(int fd, int addr, unsigned char *buf, int bytes);
int     i2c_write_page(int fd, int addr, unsigned char *data, int bytes);
int     i2c_write_byte(int fd, int addr, unsigned char *data, int bytes);
int     eeprom_write(unsigned char *data, int addr, int size);
int     eeprom_read(unsigned char *buf, int bytes, int addr);

#else

#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#define EEPROM_FILE "/sys/bus/i2c/devices/0-0050/eeprom"

/*  
#define __DEBUG
#ifdef __DEBUG
#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) do{} while(0)
#endif
*/

int eeprom_write(unsigned char *str, int offset, int bytes);
int eeprom_read(unsigned char *buf, int byte_length, int offset);

#endif
