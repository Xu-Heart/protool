#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cJSON.h"

#define BUFSIZE     16
#define MAC_SIZE    6
#define SN_SIZE     32
#define MAC_OFFSET  0
#define SN_OFFSET   40
#define EEPROM_FILE "/sys/bus/i2c/devices/0-0050/eeprom"

#define ERR_CRE_SOCK    2
#define ERR_W_SN        3
#define ERR_W_MAC       4
#define ERR_PARSE       5
#define ERR_EEPROM_W    6
#define ERR_CJSON_PKG   7
#define ERR_EEPROM_R    8

#define SEND_CLI     20 

static int     server_create_sock (int port,socklen_t len);
static void    print_usage(char *progname);
static int     mac_sn_cJSON_parse(char *str,char *buf,int size);
static int     write_sn(char *sn_value);
static int     write_mac(char *mac_value);
static int     read_sn(char *str);
static int     read_mac(char *str);
static int     eeprom_write(char *str,int offset);
static int     eeprom_read(char *buf,int length,int type);
static int     mac_sn_cJSON_pkg(char *mac_buf,char *sn_buf,char *json_str);


static int  g_stop = 0;

int main(int argc, char **argv)
{
	int                  rv = -1;
	socklen_t            len;
	int                  port = 0;
	int                  ch;
    int                  s_cre_sock = -1;

       	struct option        opts[] = {
		{"port", required_argument, NULL, 'p'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	while( (ch=getopt_long(argc, argv, "p:h", opts, NULL)) != -1 ) 
	{
		switch(ch)
		{
			case 'p':
				port=atoi(optarg);
				break;

			case 'h':
				print_usage(argv[0]);
				return 0;
		}

	}

	if( !port )
	{
		print_usage(argv[0]);
		return -1;
	}

    if((s_cre_sock = server_create_sock(port,len)) < 0)
    {
        return -ERR_CRE_SOCK;
    }

    return 0;

}


static int server_create_sock (int port,socklen_t len)
{
    int                     sockfd = -1;
    int                     on = 1;
    struct sockaddr_in      servaddr;
    struct sockaddr_in      cliaddr;
    int                     rv = -1;
    int                     w_cli_rv = -1;
    int                     i = 2;
    int                     clifd = -1;
    char                    buf[128];
    int                     parse_rv = -1;
    char                    *str = NULL;
    char                    json[128];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("create socket failure:%s\n",strerror(errno));
        return -2;
    }
    printf("create socket [%d] successful!\n",sockfd);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    rv = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    if(rv < 0)
    {
        printf("Socket[%d] bind on port[%d] failure: %s\n",sockfd, port, strerror(errno));
        close(sockfd);
        return -3;
    }
    printf("sockfd[%d] bind on port[%d] successful!\n",sockfd,port); 

    listen(sockfd,13);
    printf("listening on port %d\n",port);
    
    while(!g_stop)
    {
        printf("Start accept new client incoming...\n");

        clifd=accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        if(clifd < 0)
        {
            printf("Accept new client failure: %s\n", strerror(errno));
            return -4;
        }
        printf("Accept new client[%s:%d] successfully\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

        memset(buf, 0, sizeof(buf));
        rv=read(clifd, buf, sizeof(buf));
        if( rv < 0)
        {
            printf("Read data from client sockfd[%d] failure: %s\n", clifd, strerror(errno));
            close(clifd);
        }
        else if( rv == 0)
        {
            printf("Socket[%d] get disconnected\n", clifd);
            close(clifd);
        }
        else if( rv > 0 )
        {
            printf("Read %d bytes data from client: %s\n",rv, buf);

            /*parse cJSON format data*/
            printf("before cJSON parse\n");
            memset(json,0,sizeof(json));

            parse_rv = mac_sn_cJSON_parse(buf,json,sizeof(json));

            if((parse_rv < 0))
            {
                close(clifd);
                continue;
                //return -ERR_PARSE;
            }

            if(parse_rv == SEND_CLI)
            {
                printf("%s\n",json);
                if((w_cli_rv = write(clifd,json,strlen(json))) <= 0)
                {
                    printf("send data to client failure:%s\n",strerror(errno));
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
	printf("-h(--Help): print this help information.\n");

	return ;
}

/*parse JSON format data from client*/
/* if success ,return 0,else,return negative */
static int mac_sn_cJSON_parse(char *str,char *json,int size)
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
    if(NULL == sys_root)
    {
        printf("cJSON format data parse failure!\n");
        goto err1;
    }

    cJSON *cmd = cJSON_GetObjectItem(sys_root,"cmd");
    if(NULL == cmd)
    {
        printf("cJSON get object item failure!\n");
        goto err2;
    }
    ptr_cmd_value = cmd_buf;
    ptr_cmd_value = cmd->valuestring;
    

    cJSON *data = cJSON_GetObjectItem(sys_root,"data");
    if(NULL == data)
    {
        goto err2;
    }
    
    cJSON *sn = cJSON_GetObjectItem(data,"sn");
    if(sn != NULL)
    {
        ptr_sn_value = sn_buf;
        ptr_sn_value = sn->valuestring;
    }

    cJSON *mac = cJSON_GetObjectItem(data,"mac");  
    if(mac != NULL)
    {
        ptr_mac_value = mac_buf;
        ptr_mac_value = mac->valuestring;
    }      


    cJSON *read_type = cJSON_GetObjectItem(data,"read_type");
    if(read_type != NULL)
    {
        ptr_read_type = read_type->valuestring;
    }
    
    printf("cmd = %s, sn = %s, mac = %s,read_type = %s\r\n", ptr_cmd_value, ptr_sn_value, ptr_mac_value,ptr_read_type);

    /*justice cmd is "write" or "read"*/
    if(!strncmp(ptr_cmd_value,"write",strlen("write")))
    {

        if(ptr_sn_value != NULL)
        {
           if((w_sn_rv =  write_sn(ptr_sn_value)) < 0)
           {
               return -ERR_W_SN; 
           }
        }

        if(ptr_mac_value != NULL)
        {
            if((w_mac_rv = write_mac(ptr_mac_value)) < 0)
            {
                return -ERR_W_MAC;
            }
        }
    }

    /*when cmd is read,read mac and sn from eeprom*/
    if(!strncmp(ptr_cmd_value,"read",strlen("read")))
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


        printf("ptr_read_type = %s\n",ptr_read_type);

        memset(r_sn_buf,0,sizeof(r_sn_buf));
        memset(r_mac_buf,0,sizeof(r_mac_buf));
        memset(str_mac,0,sizeof(str_mac));
        memset(json_buf,0,sizeof(json_buf));

        if(ptr_read_type != NULL)
        {
            /*read SN from eeprom*/
            if(strncmp(ptr_read_type,"sn",strlen(ptr_read_type)) == 0)
            {
                if((rv_sn = read_sn(r_sn_buf)) == 0)
                {
                    printf("%s\n",r_sn_buf);
                }
                else if(rv_sn < 0)
                {
                    printf("read sn failure:%s\n",strerror(errno));
                }
            }

            /* read MAC from eeprom */
            if(strncmp(ptr_read_type,"mac",strlen(ptr_read_type)) == 0)
            {

                if((rv_mac = read_mac(r_mac_buf)) == 0)
                {
                    printf("\nread mac OK!\n");

                    /*convert hex MAC to string MAC */ 
                    snprintf(str_mac,18,"%02x:%02x:%02x:%02x:%02x:%02x",r_mac_buf[0],r_mac_buf[1],r_mac_buf[2],r_mac_buf[3],r_mac_buf[4],r_mac_buf[5]);

                }
                else if(rv_mac < 0)
                {
                    printf("read MAC failure:%s\n",strerror(errno));
                }
            }

            /* package data as JSON format */
            if((json_rv = mac_sn_cJSON_pkg(str_mac,r_sn_buf,json_buf)) < 0)
            {
                return -ERR_CJSON_PKG;
            }
            printf("strlen(json_buf):%d\n",strlen(json_buf));
            strncpy(json,json_buf,strlen(json_buf));

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

static int write_mac(char *mac_value)
{
    int             i = 0;
    unsigned char   buf[MAC_SIZE];
    char            *ptr = NULL;
    int             offset = -1;
    int             fd = -1;
    int             rv = -1;
    int             w_mac_rv = -1;

    printf("at write mac\n");
    printf("mac_value:%s\n",mac_value);
      
    for(i = 0;i < 6;i++)
    {
        buf[i] = strtol(mac_value,&ptr,16);
        printf("%#x ",buf[i]);
        mac_value += 3;
    }
    printf("\n");
    printf("debug:%02x\n",buf[0]);
    
    if((w_mac_rv = eeprom_write(buf,MAC_OFFSET)) < 0)
    {
        return -ERR_EEPROM_W;
    }
    printf("write mac to eeprom success!\n");

    return 0;
}

/* this function write sn to eeprom ,if success,return 0,else return nagative */
static int write_sn(char *sn_value)
{
    int     rv = -1;
    int     offset = -1;

    printf("sn_value:%s\n",sn_value);
    if((rv =  eeprom_write(sn_value,SN_OFFSET)) < 0)
    {
        return -ERR_EEPROM_W;
    }
    printf("write sn to eeprom success!\n");

    return 0;
}

static int eeprom_write(char *str,int offset)
{
    int     fd = -1;
    int     rv = -1;

    if((fd  = (open(EEPROM_FILE,O_RDWR,0666))) < 0)
    {
        printf("open eeprom file failure:%s\n",strerror(errno));
        return -1;
    }

    lseek(fd,offset,SEEK_SET);

    if((rv = write(fd,str,strlen(str))) < 0)
    {
        printf("write sn to eeprom failure!:%s\n",strerror(errno));
        close(fd);
        return -2;
    }

    printf("debug:write ok\n");
    close(fd);
    return 0;
}

/* this function read SN from eeprom */
static int read_sn(char *sn)
{
    int         sn_length = 11;
    int         rv = -1;

    printf("at read_sn\n");

    if((rv = eeprom_read(sn,sn_length,SN_OFFSET)) < 0)
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

    printf("at read mac: \n");

    if((rv = eeprom_read(mac,mac_length,MAC_OFFSET)) < 0)
    {
        return -ERR_EEPROM_R;
        
    }

    return 0;
}

int eeprom_read(char *buf,int length,int type)
{
    int             fd = -1;
    int             rv = -1;
    int             i;

    printf("length  = %d\n",length);

    if((fd  = (open(EEPROM_FILE,O_RDWR,0666))) < 0)
    {
        printf("open eeprom file failure:%s\n",strerror(errno));
        return -1;
    }

    lseek(fd,type,SEEK_SET);
    printf("debug:eerpom read 1\n");
    memset(buf,0,sizeof(buf));
    printf("debug:eeprom read 2\n");

    if((rv = read(fd,buf,length) < 0))
    {
        printf("read from eeprom failure!:%s\n",strerror(errno));
        close(fd);
        return -2;
    }

    if(length == 11)
    {
        buf[10] = '\0';
        printf("read sn:%s\n",buf);
    }
    else if(length == 7)
    {
        printf("read mac: ");
        for(i = 0;i < 6;i++)
        {
            printf("%02x ",buf[i]);
        }
        buf[6] = '\0';
    }

    close(fd);
    return 0;

}

int     mac_sn_cJSON_pkg(char *mac_buf,char *sn_buf,char *json_str)
{
    char    *json_pkg = NULL;
    int     json_rv = -1;

    cJSON *root = cJSON_CreateObject();

    if(NULL == root)
    {
        return -1;
    }
    cJSON_AddStringToObject(root,"cmd","send_cli");

    cJSON *data = cJSON_CreateObject();
    if(NULL == data)
    {
        goto err1;
    }

    printf("%s\n",mac_buf);
    printf("%x\n",(unsigned int)(*mac_buf));
    printf("%s\n",sn_buf);
    printf("%x\n",(unsigned int)(*sn_buf));

    if(*(mac_buf) != 0)
    {
        cJSON_AddStringToObject(data,"mac",(const char *)mac_buf);
    }

    if(*(sn_buf) != 0 )
    {
        cJSON_AddStringToObject(data,"sn",(const char *)sn_buf);
    }

    cJSON_AddItemToObject(root,"data",data);
    json_pkg = cJSON_PrintUnformatted(root);

    if(strncpy(json_str,json_pkg,strlen(json_pkg)) == NULL)
    {
        return -2;
    }

    printf("json_pkg : %s\n",json_pkg);
    printf("json_str : %s\n",json_str);

    return 0;

err1:
    cJSON_Delete(root);
    return -3;

}
