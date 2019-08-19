/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  main.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2019年08月18日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月18日 19时43分20秒"
 *                 
 ********************************************************************************/
#include "main.h"

struct option opts[] = {
    {"ipaddr", required_argument, NULL, 'i'},
    {"send", no_argument, NULL, 's'},
    {"recv", no_argument, NULL, 'r'},
    {"port", required_argument, NULL, 'p'},
    {"MAC", required_argument, NULL, 'M'},
    {"SN", required_argument, NULL, 'S' },
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

int main(int argc, char **argv)
{
    int         ch = -1;
    int         port = -1;
    int         rv = -1;
    int         flag = 0;
    int         sockfd = -1;
    int         type = NO_SPECIFY;
    int         i = 0;
    char        buf[256];
    char        *ip = NULL;
    char        *mac = NULL;
    char        *sn = NULL;
    char        json_buf[128];
    char        mac_value[32];
    char        sn_value[32];

    /* command argument parse  */
    while ((ch = getopt_long(argc, argv, "i:p:srM:S:h", opts, NULL)) != -1)
    {
        switch(ch)
        {
            case 'i' :
                ip = optarg;
                break;
            case 'p' :
                port = atoi(optarg);
                break;
            case 's' :
                type = WRITE;
                break;
            case 'r' :
                type = READ;
                break;
            case 'M' :
                mac = optarg;
                break;
            case 'S' :
                sn = optarg;
                break;
            case 'h' :
                print_usage(argv[0]);
                break;
            default  :
                break;
        }
    }

    if (!ip || !port || (type == NO_SPECIFY))
    {
        print_usage(argv[0]);
        return -1;
    }

    if ((type==WRITE) && ((mac==NULL)&&(sn == NULL)))
    {
        printf("option '-s' must be used with option '-M' or '-S'\n");
        return -1;
    }

    if(type == WRITE)
    {
        if((mac != NULL) && (strlen(mac) != MAC_LENGTH))
        {
            printf("mac length must 17 bytes! \n");

            return -1;
        }
        if(mac != NULL)
        {/*  mac must hex,if mac not hex ,program return errno  */
            for(i=0; i<MAC_LENGTH; i++)
            {
                if((*(mac+i)) < '0')
                {
                    printf("mac format error!\n");
                    return -1;
                }
                else if(((*(mac + i)) > ':') && ((*(mac + i)) < 'A'))
                {
                    printf("mac format error!\n");
                    return -1;
                }
                else if(((*(mac+i)) > 'F') && ((*(mac+i)) < 'a'))
                {
                    printf("mac format error!\n");
                    return -1;
                }
                else if((*(mac + i)) > 'f')
                {
                    printf("mac format error!\n");
                    return -1;
                }
            }
        }

        if((sn != NULL) && (strlen(sn) != SN_LENGTH))
        {           
            printf("sn length must 14 bytes! \n");

            return -1;
        }     

        /* look database record, if the record has existed in database, the record can't be transmit repeat*/
        printf("now,check record in database! \n");
        look_record_database(mac,sn);
    }

    /* packet cmd data as JSON format */
    memset(json_buf, 0, sizeof(json_buf));
    if((rv = pack_json_msg(mac, sn, json_buf, type)) < 0)
    {       
        return -2;
    }   

    if ((rv = connect_server(ip, port, &sockfd)) != 0)
    {       
        return -3;
    }       

    if ((rv = write(sockfd, json_buf, strlen(json_buf))) <= 0)
    {
        printf("write data to server failure:%s\n", strerror(errno));

        close(sockfd);
        return -4;
    }
    printf("send data to server success: %s\n", json_buf);
    
    /*  read reply infomation from server */
    memset(buf, 0, sizeof(buf));
    if((rv = read(sockfd, buf, sizeof(buf))) <= 0)
    {
        printf("read data from server failure:%s\n", strerror(errno));
        close(sockfd);
        return -4;
    }
    printf("read data from server: %s,next parse the data!\n", buf);

     /* if specify option '-r'(--read), parse the JSON of MAC and SN from server  */
    parse_json_msg(buf, rv, &flag, sn_value, mac_value);

    if (flag == 1)
    {
        printf("record the note(mac,sn) to database! \n");

        /* set database*/ 
        set_database(mac, sn);
    }

    DEBUG("\n---------------\nat main()\n");
    printf("client exit\n");
    return 0;
}

void print_usage(char *progname)
{
    printf("%s usage: \n",progname);
    printf("-M(--MAC):\tsend MAC to server,must be used with option '-s',specify -M(--MAC)\n");
    printf("-S(--SN):\tsend SN to server,must be used with option '-s',specify -S(--SN)\n");
    printf("-s(--send):\tsend MAC or SN to server,must be used with option '-M' or '-s' specify -s(--send)\n");
    printf("-r(--recv):\trequest to recv MAC and SN from server\n");
    printf("-p(--port):\tsepcify the port that client will to connect server's port\n");
    printf("-i(--ipaddr):\tsepcify the ipaddr that client will to connect server's ipaddress\n");
    printf("-h(--help):\tprint help information\n");

    return ;
}











