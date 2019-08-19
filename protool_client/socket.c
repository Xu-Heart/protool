/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2019年08月18日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月18日 20时01分28秒"
 *                 
 ********************************************************************************/

#include "socket.h"

/* create sockfd to connect server and net io  */
int connect_server(char *ip, int port, int *fd)
{
    int                     rv = -1;
    int                     sockfd = -1;
    struct sockaddr_in      servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);             //创建套接字

    if (sockfd < 0)
    {
        printf("create sockfd [%d] failure:%s\n", sockfd, strerror(errno));
        return -1;
    }
    printf("creat sockfd [%d] successfully\n", sockfd);
    printf("creat sockfd [%d] successfully\n", sockfd);
    memset(&servaddr, 0, sizeof(servaddr));               //填充servaddr结构体
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_aton(ip, &servaddr.sin_addr);

    if ((rv = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        DEBUG("rv = %d\n", rv);
        printf("connect to server [%s:%d] error:%s\n", ip, port, strerror(errno));
        close(sockfd);
        return -5;
    }
    else
    {
        printf("connect to server[%s:%d] successfully\n", ip, port);
        *fd = sockfd;
    }

    return 0;
}
