/********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2019年08月18日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月18日 21时20分59秒"
 *                 
 ********************************************************************************/
#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>         
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef __DEBUG
#define DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...) do{} while(0)
#endif

int connect_server(char *ip, int port, int *fd);

#endif
