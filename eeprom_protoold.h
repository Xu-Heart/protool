/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  eeprom_protool.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2019年07月24日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年07月24日 17时24分52秒"
 *                 
 ********************************************************************************/

#ifndef EEPROM_PROTOOLD_H
#define EEPROM_PROTOOLD_H

#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#define EEPROM_FILE "/sys/bus/i2c/devices/0-0050/eeprom"

int eeprom_write(char *str, int offset);
int eeprom_read(char *buf, int byte_length, int offset);

#endif
