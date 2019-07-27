/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  protoold.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2019年07月22日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年07月22日 12时50分03秒"
 *                 
 ********************************************************************************/
#ifndef     PROTOOLD_H
#define     PROTOOLD_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cJSON.h"
#include "eeprom_protoold.h"

#define BUFSIZE     16
#define MAC_SIZE    6
#define MAC_BYTES   7
#define SN_BYTES    11
#define SN_SIZE     32
#define MAC_OFFSET  0
#define SN_OFFSET   40

#define ERR_CRE_SOCK    2
#define ERR_W_SN        3
#define ERR_W_MAC       4
#define ERR_PARSE       5
#define ERR_EEPROM_W    6
#define ERR_CJSON_PKG   7
#define ERR_EEPROM_R    8

#define SEND_CLI     20 
#define PATHNAME_SN     "/etc/eeprom/SN"
#define PATHNAME_MAC    "/etc/eeprom/MAC"

static int     server_create_sock (int port,socklen_t len);
static void    print_usage(char *progname);
static int     mac_sn_cJSON_parse(char *str,char *buf,int size);
static int     write_sn(char *sn_value);
static int     write_mac(char *mac_value);
static int     read_sn(char *str);
static int     read_mac(char *str);
int            update_file(char *file, char *vlaue);
static int     mac_sn_cJSON_pkg(char *mac_buf,char *sn_buf,char *json_str);

#endif
