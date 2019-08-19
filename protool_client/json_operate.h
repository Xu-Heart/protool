/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  json_operate.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2019年08月18日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月18日 21时27分19秒"
 *                 
 ********************************************************************************/

#ifndef JSON_OPERATE_H
#define JSON_OPERATE_H

#include <stdio.h>
#include <string.h>

#include "cJSON.h"

#define WRITE        1
#define READ         0

#ifdef __DEBUG
#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) do{} while(0)
#endif

int pack_json_msg(char *mac, char *sn, char *json_buf, int type);
int parse_json_msg(char *buf, int size, int *type, char *sn_value, char *mac_value );

#endif
