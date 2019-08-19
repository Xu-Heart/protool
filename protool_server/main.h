/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  main.h
 *    Description:  This head file for main.c
 *
 *        Version:  1.0.0(2019年08月17日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月17日 14时06分49秒"
 *                 
 ********************************************************************************/
#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "socket.h"
#include "json_operate.h"
#include "eeprom_operate.h"

#define MAC_LENGTH      17
#define SN_LENGTH       14
#define HEX_MAC_BYTES   6
#define MAC_BYTES       20
#define MAC_OFFSET      0
#define SN_OFFSET       40


#define ERR_CRE_SOCK    2
#define ERR_W_SN        3
#define ERR_W_MAC       4
#define ERR_PARSE       5
#define ERR_EEPROM_W    6
#define ERR_CJSON_PKG   7
#define ERR_EEPROM_R    8

//#define __DEBUG
#ifdef __DEBUG
#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) do{} while(0)
#endif

void print_usage(char *progname);

#endif
