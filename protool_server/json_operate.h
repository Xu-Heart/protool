/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  json_operate.h
 *    Description:  This head file for json_operate.c
 *
 *        Version:  1.0.0(2019年08月16日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月16日 15时44分24秒"
 *                 
 ********************************************************************************/

#include<stdio.h>
#include<string.h>
#include"cJSON.h"

#define READ         0
#define WRITE        1

int     parse_json_msg(char *buf, int size, int *type, char *sn_value, char *mac_value);
int     pack_json_msg(char *mac_buf, char *sn_buf, char *json, int type);

//#define __DEBUG
#ifdef __DEBUG
#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) do{} while(0)
#endif

