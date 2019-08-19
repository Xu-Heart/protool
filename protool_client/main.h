/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  main.h
 *    Description:  This head file for main.c
 *
 *        Version:  1.0.0(2019年08月18日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月18日 21时10分53秒"
 *                 
 ********************************************************************************/
#ifndef MAIN_H
#define MAIN_H

#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite_database.h"
#include "json_operate.h"
#include "socket.h"


#define WRITE        1
#define READ         0
#define NO_SPECIFY  -1
#define MAC_LENGTH  17
#define SN_LENGTH   14

//define __DEBUG
#ifdef __DEBUG
#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) do{} while(0)
#endif

void print_usage(char *progname);

#endif
