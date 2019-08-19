/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2019年08月16日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月16日 14时38分24秒"
 *                 
 ********************************************************************************/

#include "socket.h"

int server_create_sock(int port, int *fd)
{
    int                     sockfd = -1;
    int                     on = 1;
    int                     rv = -1;
    struct sockaddr_in      servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("create socket failure:%s\n", strerror(errno));
        return -2;
    }
    printf("create socket [%d] successful!\n", sockfd);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    rv = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (rv < 0)
    {
        printf("Socket[%d] bind on port[%d] failure: %s\n", sockfd, port, strerror(errno));
        close(sockfd);
        return -3;
    }
    printf("sockfd[%d] bind on port[%d] successful!\n", sockfd, port);

    listen(sockfd, 13);
    printf("listening on port %d\n", port);
    *fd = sockfd;

    return 0;
}
