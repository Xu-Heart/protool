/* **********************************************************************************
 *
 *           Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                        All rights reserved.
 *    
 *            Filename:  protoold.c
 *         Description:  This file 
 *
 *             Version:  1.0.0(2019年07月22日)
 *              Author:  XuHansong <577457540@qq.com>
 *           ChangeLog:  1, Release initial version on "2019年07月18日 9时50分03秒"
 *
 ************************************************************************************/
#include"protoold.h"

static int  g_stop = 0;

int main(int argc, char **argv)
{
	int                  rv = -1;
	int                  port = 0;
	int                  ch;
    int                  create_sock = -1;
    int                  Read = 0;
    int                  Write = 0;
    int                  sockfd = -1;
    int                  clifd = -1;
    int                  type = -1;
    int                  log_fd = -1;
    char                 *mac = NULL;
    char                 *sn = NULL;
    char                 mac_backup[20];
    char                 *which = NULL;
    char                 buf[128];
    char                 json[128];
    char                 sn_value[32];
    char                 mac_value[32];
    char                 sn_buf[32];
    char                 mac_buf[32];
    struct sockaddr_in   cliaddr;
	socklen_t            len;

    struct option        opts[] = {
		{"port", required_argument, NULL, 'p'},
		{"read", required_argument, NULL, 'r'},
		{"write",no_argument, NULL, 'w'},
		{"MAC", required_argument, NULL, 'M'},
		{"SN", required_argument, NULL, 'S'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	while ((ch=getopt_long(argc, argv, "p:wr:M:S:", opts, NULL)) != -1) 
	{
		switch(ch)
		{
            case 'p' : port = atoi(optarg); break;
            case 'r' : {Read = 1; which = optarg; }; break;
            case 'w' : Write = 1; break;
            case 'M' : mac = optarg; break;
            case 'S' : sn = optarg; break;
			case 'h' : print_usage(argv[0]); break;
            default  : break;
		}
	}

    DEBUG("-------------\n");

	if ((!port) && (!Write) && (!Read))
	{
		print_usage(argv[0]);
		return -1;
	}

    if(port == 0)
    {
        if(Read == 1)
        {
            memset(buf, 0, sizeof(buf));

            if((strncmp("mac", which, 3) == 0))
            {
                if((read_mac(buf, MAC_BYTES)) != 0)
                {
                    printf("read mac failure!\n");
                    return -1;
                }
                DEBUG("read mac OK!\nmac: %s\n", buf);
                printf("mac=%s\n",buf);
            }

            if((strncmp("sn", which, 2)) == 0) 
            {
                if((read_sn(buf, SN_BYTES)) != 0)
                {
                    printf("read sn failure: %s\n", strerror(errno));
                    return -2;
                }
                printf("sn=%s\n", buf);
            }
        }
        
        if (Write == 1)
        {
            if (mac != NULL)
            {
                printf("999999\n");
                memset(mac_backup, 0, sizeof(mac_backup));
                strncpy(mac_backup, mac, strlen(mac));

                if((write_mac(mac, strlen(mac))) != 0)
                {
                    printf("write mac failure!\n");
                    return -3;
                }
            }

            if (sn != NULL)
            {
                if((write_sn(sn, strlen(sn))) != 0)
                {
                    printf("write sn failure!\n");
                    return -4;
                }
            }
        }

        return 0;
    }

    /* if the program as the socket server,run in background */
    /*  
    log_fd = open("protoold.log", O_CREAT|O_RDWR,0666);

    if (log_fd > 0)
    {
        printf("--------------\n");
        dup2(log_fd, STDOUT_FILENO);
        dup2(log_fd, STDERR_FILENO);
        daemon(1,1);
    }
    else
    {
        printf("open file failure: %s\n", strerror(errno));
        return -3;
    }
    */

    /* srever create socket */
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
        memset(mac_buf, 0,sizeof(mac_buf));
        memset(sn_buf, 0,sizeof(sn_buf));

        /* server and client start communication */
        if((rv = read(clifd, buf, sizeof(buf))) <= 0)
        {
            close(clifd);
            continue;
        }

        /* server parse JSON format data from client */
        parse_json_msg(buf, rv, &type, sn_value, mac_value);

        /* if client command to write to eeprom, judge write "MAC" or "SN"*/
        if(type == WRITE)
        {
            if((sn_value != NULL) && (strlen(sn_value)) == 10)
            {
                if((write_sn(sn_value, strlen(sn_value))) < 0)
                {
                    strncpy(sn_buf, "NAK", strlen("NAK"));
                }
                else
                {
                    strncpy(sn_buf, "ACK", strlen("ACK"));
                }
            }

            if(((*(mac_value)) != 0))
            {
                if((write_mac(mac_value, strlen(mac_value))) < 0)
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
            /* if client command to read ,judge read "MAC" or "SN" */
            if((read_sn(sn_buf, SN_BYTES)) < 0)
            {
                printf("read sn failure:%s\n", strerror(errno));

            }
            if((read_mac(mac_buf, MAC_BYTES)) < 0)
            {
                printf("read mac failure:%s\n", strerror(errno));
            }
        }

        /* result of read or write to pack as JSON format */
        pack_json_msg(mac_buf, sn_buf, json, type);

        /*result of write or read to reply client */ 
        if((rv = write(clifd, json, strlen(json))) <= 0)
        {
            printf("write to client failure: %s\n", strerror(errno));
            close(clifd);
        }
        printf("write to client: %s\n", json);
    }

    return 0;
}

static int server_create_sock(int port, int *fd)
{
    int                     sockfd = -1;
    int                     on = 1;
    int                     rv = -1;
    int                     w_cli_rv = -1;
    int                     i = 2;
    int                     clifd = -1;
    int                     parse_rv = -1;
    char                    buf[128];
    char                    *str = NULL;
    char                    json[128];
    socklen_t               len;
    struct sockaddr_in      servaddr;
    struct sockaddr_in      cliaddr;

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

static void print_usage(char *progname)
{
	printf("%s usage: \n", progname);
	printf("-p(--port): sepcify server listen port.\n");
	printf("-w(--write): write MAC or SN to eeprom.\n");
	printf("-r(--port): read MAC or SN from eeprom.\n");
	printf("-M(--MAC): sepcify the MAC that write.\n");
	printf("-S(--SN): sepcify the SN that write.\n");
	printf("-h(--Help): print this help information.\n");

	return ;
}

/*parse JSON format data from client*/
/* if success ,return 0,else,return negative */
static int parse_json_msg(char *buf, int size, int *type, char *sn_value, char *mac_value )
{
    char    *ptr_mac_value = NULL;
    char    *ptr_sn_value = NULL;
    char    *ptr_cmd_value = NULL;
    char    *ptr_whichype = NULL;
    int     strcmp_rv = -1;
    int     w_sn_rv = -1;
    int     w_mac_rv = -1;
    cJSON   *root = NULL;

    cJSON *sys_root = cJSON_Parse(buf);
    if (NULL == sys_root)
    {
        printf("cJSON format data parse failure!\n");
        goto err1;
    }

    cJSON *cmd = cJSON_GetObjectItem(sys_root, "cmd");
    if (NULL == cmd)
    {
        printf("cJSON get object item failure!\n");
        goto err2;
    }
    ptr_cmd_value = cmd->valuestring;
    
    cJSON *data = cJSON_GetObjectItem(sys_root, "data");
    if (NULL == data)
    {
        goto err2;
    }
    
    cJSON *sn = cJSON_GetObjectItem(data, "sn");
    if (sn != NULL)
    {
        ptr_sn_value = sn->valuestring;
    }

    cJSON *mac = cJSON_GetObjectItem(data, "mac");  
    if (mac != NULL)
    {
        ptr_mac_value = mac->valuestring;
    }      

    printf("cmd = %s, sn = %s, mac = %s\n", ptr_cmd_value, ptr_sn_value, ptr_mac_value, ptr_whichype);

    /*justice cmd is "write" or "read"*/
    if (!strncmp(ptr_cmd_value, "write", strlen("write")))
    {
        *type = WRITE;

        if (ptr_sn_value != NULL)
        {
            strncpy(sn_value, ptr_sn_value, strlen(ptr_sn_value));
        }

        if (ptr_mac_value != NULL)
        {
            strncpy(mac_value, ptr_mac_value, strlen(ptr_mac_value));
        }
    }

    /*when cmd is read,read mac and sn from eeprom*/
    if (!strncmp(ptr_cmd_value, "read", strlen("read")))
    {
        *type = READ;
    }

err2:
    cJSON_Delete(sys_root);

err1:
    //free(str);
    return 0;
}

/* the function write mac to eeprom */
static int write_mac(char *mac_value, int bytes)
{
    int             i = 0;
    int             offset = -1;
    int             fd = -1;
    int             rv = -1;
    char            mac_value_bake[32];
    char            *ptr = NULL;
    char            mac_ini_buf[MAC_INI_LENGTH];
    unsigned char   buf[MAC_SIZE];


    memset(mac_value_bake, 0, sizeof(mac_value_bake));
    if(mac_value != NULL)
    {
        strncpy(mac_value_bake, mac_value, strlen(mac_value));
    }
    //printf("mac_value_bake: %s\n", mac_value_bake);
    printf("mac_value: %s\n", mac_value);
      
    for (i=0; i<6; i++)
    {
        buf[i] = strtol(mac_value, &ptr, 16);
        mac_value += 3;
    }
    printf("debug:%02x\n", buf[0]);
    
    if ((rv = eeprom_write(buf,MAC_OFFSET)) < 0)
    {
        return -ERR_EEPROM_W;
    }
    printf("write mac to eeprom success!\n");

    /*update the file "/etc/.hwinfo" */
    /*  
    memset(mac_ini_buf, 0,sizeof(mac_ini_buf));
    snprintf(mac_ini_buf, MAC_INI_LENGTH, "mac=%s", mac_value_bake);
    printf("%s\n", mac_ini_buf);
    if ((update_file(PATHNAME_HWINFO, mac_ini_buf)) != 0)
    {
        printf("update file failure!\n");
    }
    */

    return 0;
}

/* this function write sn to eeprom ,if success,return 0,else return nagative */
static int write_sn(char *sn_value, int bytes)
{
    int     rv = -1;
    int     offset = -1;

    printf("sn_value:%s\n", sn_value);
    if ((rv =  eeprom_write(sn_value,SN_OFFSET)) < 0)
    {
        return -ERR_EEPROM_W;
    }
    printf("write sn to eeprom success!\n");

    /* update the file '/etc/eeprom/SN' */
    /*  
    memset(sn_ini_buf, 0,sizeof(sn_ini_buf));
    snprintf(sn_ini_buf, SN_INI_LENGTH, "sn=%s", sn_value);
    if ((update_file(PATHNAME_HWINFO, sn_ini_buf)) != 0)
    {
        printf("update file '/etc/.hwinfo' failure!\n");
    }
    */

    return 0;
}

/* this function read SN from eeprom */
static int read_sn(char *sn, int sn_length)
{
    int         rv = -1;

    if ((rv = eeprom_read(sn, sn_length, SN_OFFSET)) < 0)
    {
        return -ERR_EEPROM_R;
    }
    DEBUG("read sn ok!%s\n",sn);

    return 0;
}

static int read_mac(char *mac, int bytes)
{
    int         rv = -1;
    char        buf[7];

    if ((rv = eeprom_read(buf, 7, MAC_OFFSET)) < 0)
    {
        return -ERR_EEPROM_R;
    }

    /* convert hex MAC to string MAC */
    snprintf(mac, bytes, "%02x:%02x:%02x:%02x:%02x:%02x",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    return 0;
}

int     pack_json_msg(char *mac_buf, char *sn_buf, char *json, int type)
{
    char    *json_pkg = NULL;
    int     json_rv = -1;

    cJSON *root = cJSON_CreateObject();

    if (NULL == root)
    {
        return -1;
    }
    
    if(type == WRITE)
    {
        cJSON_AddStringToObject(root, "cmd", "reply_write");
    }
    else
    {
        cJSON_AddStringToObject(root, "cmd", "reply_read");
    }

    cJSON *data = cJSON_CreateObject();
    if (NULL == data)
    {
        goto err1;
    }

    printf("%s\n", mac_buf);
    printf("%x\n", (unsigned int)(*mac_buf));
    printf("%s\n", sn_buf);
    printf("%x\n", (unsigned int)(*sn_buf));

    if (*(mac_buf) != 0)
    {
        cJSON_AddStringToObject(data, "mac", (const char *)mac_buf);
    }

    if (*(sn_buf) != 0 )
    {
        cJSON_AddStringToObject(data, "sn", (const char *)sn_buf);
    }

    cJSON_AddItemToObject(root, "data", data);
    json_pkg = cJSON_PrintUnformatted(root);

    if (strncpy(json, json_pkg, strlen(json_pkg)) == NULL)
    {
        return -2;
    }

    printf("json_pkg: %s\n", json_pkg);
    printf("strlen(json_pkg) = %d\n", strlen(json_pkg));
    printf("json: %s\n", json);

    return 0;

err1:
    cJSON_Delete(root);
    return -3;

}

/*  
int update_file(char *file, char *value)
{
    int     fd = -1;

    //printf("file: %s\n", file);
    //printf("value: %s\n", value);

    fd = open(file, O_RDWR | O_CREAT, 0666);

    if(fd < 0)
    {
        printf("open [%s] failure: %s\n", file, strerror(errno));
        return -1;
    }

    printf("strlen(value) =%d\n",strlen(value));
    if(strlen(value) == 21)
    {
        lseek(fd, 0, SEEK_SET);
        if((write(fd, value, strlen(value))) <= 0)
        {
            printf("write data to file '%s' failure: %s\n", file, strerror(errno));
            return -2;
        }
        lseek(fd,21,SEEK_SET);
        write(fd,"\n",1);
        printf("update the file '%s' successful!\n", file);
    }
    else if(strlen(value) == 13)
    {
        lseek(fd, 22, SEEK_SET);
        if((write(fd, value, strlen(value))) <= 0)
        {           
            printf("write data to file '%s' failure: %s\n", file, strerror(errno));
            return -2;
        }     
        lseek(fd,35, SEEK_SET);
        write(fd,"\n",1);
         printf("update the file '%s' successful!\n", file);

    }
    else
    {
        printf("MAC or SN legth is error!\n");
    }

    return 0;
}
*/
