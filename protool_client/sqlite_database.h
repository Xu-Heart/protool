/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  sqlite_database.h
 *    Description:  This head file for sqlite_database.c 
 *
 *        Version:  1.0.0(2019年08月19日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月19日 09时45分38秒"
 *                 
 ********************************************************************************/

#ifndef     SQLITE_DATABASE_H
#define     SQLITE_DATABASE_H

#include <stdio.h>
#include <sqlite3.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>


#ifdef __DEBUG
#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) do{} while(0)
#endif


int     set_database(char *mac, char *sn);
int     callback(void *NotUsed, int argc, char **argv, char **azColName);
int     look_record_database(char *mac, char *sn);
int     print_record_mac(void *params, int n_column, char **column_value, char **column_name);
int     print_record_sn(void *params, int n_column, char **column_value, char **column_name);

#endif
