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
    int                  s_cre_sock = -1;
    int                  read = 0;
    int                  write = 0;
    int                  log_fd = -1;
    char                 *ptr_mac = NULL;
    char                 *ptr_mac_backup = NULL;
    char                 *ptr_sn = NULL;
	socklen_t            len;
    char                 *optstring = "p:wr:M:S:";
    char                 *read_t = NULL;
    char                 r_buf[20];

    struct option        opts[] = {
		{"port", required_argument, NULL, 'p'},
		{"read", required_argument, NULL, 'r'},
		{"write",no_argument, NULL, 'w'},
		{"MAC", required_argument, NULL, 'M'},
		{"SN", required_argument, NULL, 'S'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	while ((ch=getopt_long(argc, argv, optstring, opts, NULL)) != -1) 
	{
		switch(ch)
		{
            case 'p' : port = atoi(optarg); break;
            case 'r' : {read = 1; read_t = optarg; }; break;
            case 'w' : write = 1; break;
            case 'M' : ptr_mac = optarg; break;
            case 'S' : ptr_sn = optarg; break;
			case 'h' : print_usage(argv[0]); break;
            default  : break;
		}
	}

    ptr_mac_backup = ptr_mac;

	if ((!port) && (!write) && (!read))
	{
		print_usage(argv[0]);
		return -1;
	}

    if(port == 0)
    {
        if(read == 1)
        {
            if((strncmp("mac", read_t, 3) == 0))
            {
                if((read_mac(r_buf)) != 0)
                {
                    printf("read mac failure!\n");
                    return -1;
                }
                printf("read mac OK!\nmac: %s\n", r_buf);
            }

            if((strncmp("sn", read_t, 2)) == 0) 
            {
                if((read_sn(r_buf)) != 0)
                {
                    printf("read sn failure!\n");
                    return -2;
                }
                printf("sn: %s\n", r_buf);
            }
        }
        
        if (write == 1)
        {
            if (ptr_mac != NULL)
            {
                if((write_mac(ptr_mac)) != 0)
                {
                    printf("write mac failure!\n");
                    return -3;
                }
            }

            if (ptr_sn != NULL)
            {
                if((write_sn(ptr_sn)) != 0)
                {
                    printf("write sn failure!\n");
                    return -4;
                }
            }
        }
        return 0;
    }

    /* if the program as the socket server,run in background */
    log_fd = open("protoold.log",O_RDWR|O_CREAT,0666);

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

    if ((s_cre_sock=server_create_sock(port, len)) < 0)
    {
        return -ERR_CRE_SOCK;
    }

    return 0;
}

static int server_create_sock(int port, socklen_t len)
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
    
    while (!g_stop)
    {
        printf("Start accept new client incoming...\n");

        clifd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        if (clifd < 0)
        {
            printf("Accept new client failure: %s\n", strerror(errno));
            return -4;
        }
        printf("Accept new client[%s:%d] successfully\n", inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

        memset(buf, 0, sizeof(buf));
        rv = read(clifd, buf, sizeof(buf));
        if (rv < 0)
        {
            printf("Read data from client sockfd[%d] failure: %s\n", clifd, strerror(errno));
            close(clifd);
        }
        else if (rv == 0)
        {
            printf("Socket[%d] get disconnected\n", clifd);
            close(clifd);
        }
        else if (rv > 0)
        {
            printf("Read %d bytes data from client: %s\n", rv, buf);

            /*parse cJSON format data*/
            printf("before cJSON parse\n");
            memset(json, 0, sizeof(json));

            parse_rv = mac_sn_cJSON_parse(buf, json, sizeof(json));
            if ((parse_rv < 0))
            {
                close(clifd);
                continue;
            }
            if (parse_rv == SEND_CLI)
            {
                printf("%s\n", json);

                if ((w_cli_rv = write(clifd, json, strlen(json))) <= 0)
                {
                    printf("send data to client failure:%s\n", strerror(errno));
                    return -5;
                }
                printf("send data to client success!\n");
            }

            close(clifd);
        }
    }
    close(sockfd);
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
static int mac_sn_cJSON_parse(char *str, char *json, int size)
{
    char    *ptr_mac_value = NULL;
    char    *ptr_sn_value = NULL;
    char    *ptr_cmd_value = NULL;
    char    *ptr_read_type = NULL;
    char    cmd_buf[8];
    char    sn_buf[16];
    char    mac_buf[20];
    int     strcmp_rv = -1;
    int     w_sn_rv = -1;
    int     w_mac_rv = -1;
    cJSON   *root = NULL;

    cJSON *sys_root = cJSON_Parse(str);
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
    ptr_cmd_value = cmd_buf;
    ptr_cmd_value = cmd->valuestring;
    

    cJSON *data = cJSON_GetObjectItem(sys_root, "data");
    if (NULL == data)
    {
        goto err2;
    }
    
    cJSON *sn = cJSON_GetObjectItem(data, "sn");
    if (sn != NULL)
    {
        ptr_sn_value = sn_buf;
        ptr_sn_value = sn->valuestring;
    }

    cJSON *mac = cJSON_GetObjectItem(data, "mac");  
    if (mac != NULL)
    {
        ptr_mac_value = mac_buf;
        ptr_mac_value = mac->valuestring;
    }      

    cJSON *read_type = cJSON_GetObjectItem(data, "read_type");
    if (read_type != NULL)
    {
        ptr_read_type = read_type->valuestring;
    }
    
    printf("cmd = %s, sn = %s, mac = %s, read_type = %s\r\n", ptr_cmd_value, ptr_sn_value, ptr_mac_value, ptr_read_type);

    /*justice cmd is "write" or "read"*/
    if (!strncmp(ptr_cmd_value, "write", strlen("write")))
    {
        if (ptr_sn_value != NULL)
        {
           if ((w_sn_rv =  write_sn(ptr_sn_value)) < 0)
           {
               return -ERR_W_SN; 
           }
        }

        if (ptr_mac_value != NULL)
        {
            if ((w_mac_rv = write_mac(ptr_mac_value)) < 0)
            {
                return -ERR_W_MAC;
            }
        }
    }

    /*when cmd is read,read mac and sn from eeprom*/
    if (!strncmp(ptr_cmd_value, "read", strlen("read")))
    {
        char    r_sn_buf[11];
        char    r_mac_buf[7];
        char    str_mac[18];
        char    *str_pkg = NULL;
        char    json_buf[128];
        char    send_buf[64];   
        int     send_rv = -1;
        int     rv_mac  = -1;
        int     rv_sn = -1;
        int     json_rv = -1;

        printf("ptr_read_type = %s\n", ptr_read_type);

        memset(r_sn_buf, 0, sizeof(r_sn_buf));
        memset(r_mac_buf, 0, sizeof(r_mac_buf));
        memset(str_mac, 0, sizeof(str_mac));
        memset(json_buf, 0, sizeof(json_buf));

        if (ptr_read_type != NULL)
        {
            /*read SN from eeprom*/
            if (strncmp(ptr_read_type, "sn", strlen(ptr_read_type)) == 0)
            {
                if ((rv_sn = read_sn(r_sn_buf)) == 0)
                {
                    printf("%s\n", r_sn_buf);
                }
                else if (rv_sn < 0)
                {
                    printf("read sn failure:%s\n", strerror(errno));
                }
            }

            /* read MAC from eeprom */
            if (strncmp(ptr_read_type, "mac", strlen(ptr_read_type)) == 0)
            {
                if ((rv_mac = read_mac(str_mac)) == 0)
                {
                    printf("\nread mac OK!mac: %s\n", str_mac);
                }
                else if (rv_mac < 0)
                {
                    printf("read MAC failure:%s\n", strerror(errno));
                }
            }

            /* package data as JSON format */
            if ((json_rv = mac_sn_cJSON_pkg(str_mac, r_sn_buf, json_buf)) < 0)
            {
                return -ERR_CJSON_PKG;
            }
            printf("strlen(json_buf):%d\n", strlen(json_buf));
            strncpy(json, json_buf, strlen(json_buf));

            return SEND_CLI;
        }
        else
        {
            printf("client don't specify the argument of '-r'\n");
            return -1;
        }
    }

err2:
    cJSON_Delete(sys_root);

err1:
    //free(str);
    return 0;
}

/* the function write mac to eeprom */
static int write_mac(char *mac_value)
{
    int             i = 0;
    int             offset = -1;
    int             fd = -1;
    int             rv = -1;
    int             w_mac_rv = -1;
    char            *mac_value_bake = NULL;
    char            *ptr = NULL;
    unsigned char   buf[MAC_SIZE];

    mac_value_bake = mac_value;
    //printf("mac_value_bake: %s\n", mac_value_bake);
      
    for (i=0; i<6; i++)
    {
        buf[i] = strtol(mac_value, &ptr, 16);
        mac_value += 3;
    }
    //printf("debug:%02x\n", buf[0]);
    
    if ((w_mac_rv = eeprom_write(buf,MAC_OFFSET)) < 0)
    {
        return -ERR_EEPROM_W;
    }
    printf("write mac to eeprom success!\n");

    /*update the file "/etc/eeprom/MAC" */
    if ((update_file(PATHNAME_MAC, mac_value_bake)) != 0)
    {
        printf("update file failure!\n");
    }

    return 0;
}

/* this function write sn to eeprom ,if success,return 0,else return nagative */
static int write_sn(char *sn_value)
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
    if ((update_file(PATHNAME_SN, sn_value)) != 0)
    {
        printf("update file '/etc/eeprom/sn' failure!\n");
    }

    return 0;
}

/* this function read SN from eeprom */
static int read_sn(char *sn)
{
    int         sn_length = 11;
    int         rv = -1;

    if ((rv = eeprom_read(sn, sn_length, SN_OFFSET)) < 0)
    {
        return ERR_EEPROM_R;
    }
    printf("read sn ok!\n");

    return 0;
}

static int read_mac(char *mac)
{
    int         mac_length = 7;
    int         rv = -1;
    char        buf[7];

    if ((rv = eeprom_read(buf, mac_length, MAC_OFFSET)) < 0)
    {
        return -ERR_EEPROM_R;
    }

    /* convert hex MAC to string MAC */
    snprintf(mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    return 0;
}

int     mac_sn_cJSON_pkg(char *mac_buf, char *sn_buf, char *json_str)
{
    char    *json_pkg = NULL;
    int     json_rv = -1;

    cJSON *root = cJSON_CreateObject();

    if (NULL == root)
    {
        return -1;
    }
    cJSON_AddStringToObject(root, "cmd", "send_cli");

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

    if (strncpy(json_str, json_pkg, strlen(json_pkg)) == NULL)
    {
        return -2;
    }

    printf("json_pkg: %s\n", json_pkg);
    printf("strlen(json_pkg) = %d\n", strlen(json_pkg));
    printf("json_str: %s\n", json_str);

    return 0;

err1:
    cJSON_Delete(root);
    return -3;

}

int update_file(char *file, char *value)
{
    int     fd = -1;

    //printf("file: %s\n", file);
    //printf("value: %s\n", value);

    fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0666);

    if(fd < 0)
    {
        printf("open [%s] failure: %s\n", file, strerror(errno));
        return -1;
    }

    lseek(fd, 0, SEEK_SET);

    if((write(fd, value, strlen(value))) <= 0)
    {
        printf("write data to file '%s' failure: %s\n", file, strerror(errno));
        return -2;
    }
    printf("update the file '%s' successful!\n", file);

    return 0;
}
