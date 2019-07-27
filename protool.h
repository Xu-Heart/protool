/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  protool.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2019年07月22日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年07月22日 12时12分36秒"
 *                 
 ********************************************************************************/

#ifndef     PROTOOL_H
#define     PROTOOL_H

#include<stdio.h>
#include<getopt.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sqlite3.h>
#include<time.h>
#include"cJSON.h"

void        print_usage(char argv[0]);
int         connect_server(char *ip, int port, char *buf, int recv, int *flag);
int         mac_sn_cJSON_pkg(char *ptr_mac, char *ptr_sn, int send, int recv, 
                            char *read_type, char *json_str);
static int  callback(void *NotUsed, int argc, char **argv, char **azColName);
int         set_database(char *ptr_mac, char *ptr_sn);
int         look_record_database(char *ptr_mac, char *ptr_sn);
int         print_record_mac(void *params,int n_column,char **column_value,char **column_name);
int         print_record_sn(void *params, int n_column, char **column_value, 
                            char **column_name);

#endif
