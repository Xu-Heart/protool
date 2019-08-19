/**************************************************************************************
 * 
 *               Copyright:(C) 2019 XuHansong<577457540@qq.com>
 *                             All rights reserved.
 *               Filename:      main.c
 *               Description:   This file 
 *               Version:       1.0.0(2019年07月22日)
 *               Author:        XuHansong <577457540@qq.com>
 *               ChangeLog:     1, Release initial version on "2019年07月18日 9时50分03秒
 *         
 *************************************************************************************/
#include"main.h"

int g_stop = 0;

int main(int argc, char **argv)
{
    int                  rv = -1;
    int                  port = 0;
    int                  ch = -1;
    int                  Read = 0;
    int                  Write = 0;
    int                  sockfd = -1;
    int                  clifd = -1;
    int                  type = -1;
    int                  log_fd = -1;
    char                 *mac = NULL;
    char                 *sn = NULL;
    char                 *which = NULL;
    char                 buf[128];
    char                 json[128];
    char                 sn_value[32];
    char                 mac_value[32];
    char                 sn_buf[SN_LENGTH + 1];
    char                 mac_buf[MAC_LENGTH + 1];
    struct sockaddr_in   cliaddr;
    socklen_t            len = sizeof(struct sockaddr);

    struct option        opts[] = {
        {"port", required_argument, NULL, 'p'},
        {"read", required_argument, NULL, 'r'},
        {"write",no_argument, NULL, 'w'},
        {"MAC", required_argument, NULL, 'M'},
        {"SN", required_argument, NULL, 'S'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    /*  command line argument parse */
    while ((ch=getopt_long(argc, argv, "p:wr:M:S:", opts, NULL)) != -1)
    {
        switch(ch)
        {
            case 'p' : 
                port = atoi(optarg); 
                break;
            case 'r' : 
                Read = 1; 
                which = optarg; 
                break;
            case 'w' : 
                Write = 1; 
                break;
            case 'M' : 
                mac = optarg; break;
            case 'S' : 
                sn = optarg; break;
            case 'h' : 
                print_usage(argv[0]); 
                break;
            default  : break;
        }
    }

    /* if no specify port (-p) and  no specify option -w and  -r */
    if ((!port) && (!Write) && (!Read))
    {
        print_usage(argv[0]);
        return -1;
    }

    /* if port is 0, the program run not as socket server program */
    if (port == 0)
    {
        if (Read == 1)
        {
            memset(buf, 0, sizeof(buf));

            if ((strncmp("mac", which, strlen("mac")) == 0))
            {
                if ((read_mac(buf, MAC_BYTES)) != 0)
                {
                    printf("read mac failure!\n");
                    return -1;
                }
                DEBUG("read mac OK!\nmac: %s\n", buf);
                printf("mac=%s\n",buf);
            }

            if ((strncmp("sn", which, strlen("sn")) == 0))
            {
                if ((read_sn(buf, SN_LENGTH + 1)) != 0)
                {
                    printf("read sn failure!\n");
                    return -1;
                }
                DEBUG("read sn OK!\nsn: %s\n", buf);
                printf("sn=%s\n",buf);
            }
        }
        if (Write == 1)
        {
            if ((mac != NULL) && (strlen(mac) != MAC_LENGTH))
            {
                printf("mac length must 17 bytes! \n");

                return -1;
            }
            if (mac != NULL)
            {
                /*   mac must hex,if mac not hex ,program return errno */
                int     i;

                for (i=0; i<MAC_LENGTH; i++)
                {
                    if ((*(mac+i)) < '0')
                    {
                        printf("mac format error!\n");
                        return -1;
                    }
                    else if (((*(mac + i)) > ':') && ((*(mac + i)) < 'A'))
                    {
                        printf("mac format error!\n");
                        return -1;
                    }
                    else if (((*(mac+i)) > 'F') && ((*(mac+i)) < 'a'))
                    {
                        printf("mac format error!\n");
                        return -1;
                    }
                    else if ((*(mac + i)) > 'f')
                    {
                        printf("mac format error!\n");
                        return -1;
                    }
                }

                if ((write_mac(mac, MAC_LENGTH)) != 0)
                {
                    printf("write mac failure!\n");
                    return -3;
                }
            }

            if ((sn != NULL) && (strlen(sn) != SN_LENGTH))
            {
                printf("sn length must 14 bytes! \n");

                return -1;
            }

            if (sn != NULL)
            {           
                if ((write_sn(sn, strlen(sn))) != 0)
                {
                    printf("write sn failure!\n");
                    return -4;
                }
            }
        }

        return 0;
    }

    /*  if the program as the socket server,run in background; and print log to /tmp/protoold.log file*/
    log_fd = open("/tmp/protoold.log", O_CREAT|O_RDWR,0666);

    if (log_fd > 0)
    {
        dup2(log_fd, STDOUT_FILENO);
        dup2(log_fd, STDERR_FILENO);
        daemon(1,1);
    }
    else
    {
        printf("open file failure: %s\n", strerror(errno));
        return -3;
    }

    /*  srever create socket */
    if (( rv = server_create_sock(port, &sockfd)) < 0)
    {
        return -ERR_CRE_SOCK;
    }

    while (!g_stop)
    {
        printf("Start accept new client incoming...\n");

        clifd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        if (clifd < 0)
        {
            printf("Accept new client failure: %s\n", strerror(errno));
            return -4;
        }
        printf("Accept new client[%s:%d] successfully\n", inet_ntoa(cliaddr.sin_addr),
                ntohs(cliaddr.sin_port));

        memset(buf, 0, sizeof(buf));
        memset(sn_value, 0, sizeof(sn_value));
        memset(mac_value, 0, sizeof(mac_value));
        memset(json, 0, sizeof(json));
        memset(mac_buf, 0, sizeof(mac_buf));
        memset(sn_buf, 0, sizeof(sn_buf));

        /*  server and client start communication */
        if ((rv = read(clifd, buf, sizeof(buf))) <= 0)
        {
            close(clifd);
            continue;
        }

        /*  server parse JSON format data from client */
        parse_json_msg(buf, rv, &type, sn_value, mac_value);

        /*  if client command to write to eeprom, judge write "MAC" or "SN"*/
        if (type == WRITE)
        {
            printf("进行socket写操作\n");
            if ((sn_value != NULL) && (strlen(sn_value) == SN_LENGTH))
            {
                printf("sn:%s\n",sn_value);
                rv = write_sn(sn_value, SN_LENGTH);
                if (rv < 0)
                {
                    strncpy(sn_buf, "NAK", strlen("NAK"));
                }
                else
                {
                    strncpy(sn_buf, "ACK", strlen("ACK"));
                }
            }

            if ((mac_value != NULL) && (strlen(mac_value) == MAC_LENGTH))
            {
                //printf("需要写的MAC值:%s\n", mac_value);
                //#define TEST_MAC "FF:FF:FF:FF:FF:FE"

                rv = write_mac(mac_value, MAC_LENGTH);

                if (rv < 0)
                {
                    strncpy(mac_buf, "NAK", strlen("NAK"));
                }
                else
                {
                    strncpy(mac_buf, "ACK", strlen("ACK"));
                }
            }
        }
        else
        {   
            /*  if client command to read ,judge read "MAC" or "SN" */
            if ((read_sn(sn_buf, SN_LENGTH + 1)) < 0)
            {   
                printf("read sn failure:%s\n", strerror(errno));

            }   
            if ((read_mac(mac_buf, MAC_BYTES)) < 0)
            {   
                printf("read mac failure:%s\n", strerror(errno));
            }
        }

        /*  result of read or write to pack as JSON format */
        pack_json_msg(mac_buf, sn_buf, json, type);

        /* result of write or read to reply client */
        if ((rv = write(clifd, json, strlen(json))) <= 0)
        {
            printf("write to client failure: %s\n", strerror(errno));
            close(clifd);
        }
        printf("write to client: %s\n", json);
        close(clifd);
    }

    close(sockfd);
    close(log_fd);
    //close(log_fd);
    return 0;
}


void print_usage(char *progname)
{
        printf("%s usage: \n", progname);
        printf("-p(--port): sepcify server listen port.\n");
        printf("-w(--write): write MAC or SN to eeprom, must used with option '-S(--SN)' or '-M(--MAC' \n");
        printf("-r(--port): read MAC or SN from eeprom.\n");
        printf("-M(--MAC): sepcify the MAC that write.\n");
        printf("-S(--SN): sepcify the SN that write.\n");
        printf("-h(--Help): print help information.\n");

        return ;
}
