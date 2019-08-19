/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  eeprom_operate.h
 *    Description:  This head file is for eeprom_operate.c
 *
 *        Version:  1.0.0(2019年08月16日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月16日 15时51分38秒"
 *                 
 ********************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAC_OFFSET      0
#define SN_OFFSET       40

#define HEX_MAC_BYTES   6

#define ERR_EEPROM_W    6
#define ERR_EEPROM_R    8


//#define __DEBUG
#ifdef __DEBUG
#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) do{} while(0)
#endif

int write_mac(char *mac_value, int bytes);
int write_sn(unsigned char *sn_value, int bytes);
int read_mac(unsigned char *mac, int bytes);
int read_sn(unsigned char *sn, int sn_length);

