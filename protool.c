/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  write.c
 *    Description:  This file is protool.c 
 *                 
 *        Version:  1.0.0(2019年07月05日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年07月05日 19时13分53秒"
 *                 
 ********************************************************************************/
#include "protool.h"

static struct option opts[] = {
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
    char        buf[256];
    char        *ip = NULL;
    char        *mac = NULL;
    char        *sn = NULL;
    char        json_buf[128];
    char        mac_value[32];
    char        sn_value[32];

    DEBUG("at main\n------------\n");
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
        //printf("strlen(mac) = %d\nstrlen(sn) = %d\n", strlen(mac), strlen(sn));
        if((mac != NULL) && (strlen(mac) != MAC_LENGTH))
        {
            printf("mac length must 17 bytes! \n");

            return -1;
        }
        if(mac != NULL)
        {
            /* mac must hex,if mac not hex ,program return errno */
            int     i;

            for(i=0; i<MAC_LENGTH; i++)
            {
                if((*(mac+i)) < '0')
                {
                    printf("mac format errno!\n");
                    return -1;
                }
                else if(((*(mac + i)) > ':') && ((*(mac + i)) < 'A'))
                {
                    printf("mac format errno!\n");
                    return -1;
                }
                else if(((*(mac+i)) > 'F') && ((*(mac+i)) < 'a'))
                {
                    printf("mac format errno!\n");
                    return -1;
                }
                else if((*(mac + i)) > 'f')
                {
                    printf("mac format errno!\n");
                    return -1;
                }
            }
        }
        
        if((sn != NULL) && (strlen(sn) != SN_LENGTH))
        {
            printf("sn length must 10 bytes! \n");

            return -1;
        }

        /*look database record */
        printf("now,check record in database! \n");
        look_record_database(mac,sn);
    }

    /*pack cmd data as JSON format */
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
        return -3;
    }
    printf("send data to server success: %s\n", json_buf);
    
    /* 读取服务器端的回复信息 */
    memset(buf, 0, sizeof(buf));
    if((rv = read(sockfd, buf, sizeof(buf))) <= 0)
    {
        printf("read data from server failure:%s\n", strerror(errno));
        close(sockfd);
        return -4;
    }
    printf("read data from server: %s,next parse the data!\n", buf);

    /*if specify option '-r'(--read), parse the JSON of MAC and SN from server */ 
    parse_json_msg(buf, rv, &flag, sn_value, mac_value);

    if (flag == 1)
    {
        printf("record the note(mac,sn) to database! \n");

        /*set database*/
        set_database(mac, sn);
    }

    DEBUG("\n---------------\nat main()\n");
    printf("client exit\n");
    return 0;
}

static void print_usage(char *progname)
{
    printf("%s usage: \n",progname);
    printf("-M(--MAC):\tsend MAC to server,must be used with option '-s',specify -M(--MAC)\n");
    printf("-S(--SN):\tsend SN to server,must be used with option '-s',specify -S(--SN)\n");
    printf("-s(--send):\tsend MAC or SN to server,must be used with option '-M' or '-s', specify -s(--send)\n");
    printf("-r(--recv):\trequest to recv MAC and SN from server\n");
    printf("-p(--port):\tsepcify the port that client will to connect server's port\n");
    printf("-i(--ipaddr):\tsepcify the ipaddr that client will to connect server's ipaddress\n");
    printf("-h(--help):\tprint help information\n");

    return ;
}

/*create sockfd to connect server and net io */
static int connect_server(char *ip, int port, int *fd)
{
    int                     rv = -1;
    int                     sockfd = -1;
    struct sockaddr_in      servaddr;

    DEBUG("------------------\nat connect_server()");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);             //创建套接字
    if (sockfd < 0)
    {
        printf("create sockfd [%d] failure:%s\n", sockfd, strerror(errno));
        return -1;
    }
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

/* package mac an sn as JSON format*/
static int pack_json_msg(char *mac, char *sn, char *json_buf, int type)
{
    char    *json = NULL;
    int     rv = -1;

    cJSON *root = cJSON_CreateObject();

    if (NULL == root)
    {
        return -1;
    }
    
    if (type == WRITE)
    {
        cJSON_AddStringToObject(root, "cmd", "write");
    }
    if (type == READ )
    {
        cJSON_AddStringToObject(root, "cmd", "read");
    }

    cJSON *data = cJSON_CreateObject();
    if (NULL == data)
    {
        goto err1;
    }

    if (mac != NULL)
    {
        DEBUG("mac != NULL\n");
        cJSON_AddStringToObject(data, "mac", (const char *)mac);
    }
    
    if (sn != NULL )
    {
        DEBUG("sn != NULL\n");
        cJSON_AddStringToObject(data, "sn", (const char *)sn);
    }

    cJSON_AddItemToObject(root, "data", data);
    json = cJSON_PrintUnformatted(root);

    DEBUG("strlen(json) = %d\n", (int)strlen(json));
    strncpy(json_buf, json, strlen(json));
    DEBUG("--------debug:ptr_json:%s\n", json);
    DEBUG("--------debug:json_pkg:%s\n", json_buf);
    DEBUG("--------debug:strlen(json_buf) = %d\nsizeof(json_buf) = %d\n",   (int)strlen(json_buf),(int)sizeof(json_buf));

    return 0;

err1:
    cJSON_Delete(root);
    return -3;
}

static int set_database(char *mac, char *sn)
{
    int         rc = -1;
    char        *zErrMsg = NULL;
    sqlite3     *db = NULL;
    char        *sql = NULL;
    char        *sql_judge_table_exist = NULL;
    char        sql_buf[128];
    int         is_table_exist = 0;
    struct tm   *time_now = NULL;
    time_t      timep;
    char        time_buf[128];

    DEBUG("\n--------------------\nat set_database\n");

    /*get systime */
    time(&timep);
    time_now = localtime(&timep);

    memset(time_buf, 0, sizeof(time_buf));
    snprintf(time_buf, sizeof(time_buf), "%d-%d-%d %d:%d:%d", (time_now -> tm_year + 1900), (time_now -> tm_mon + 1), (time_now -> tm_mday), (time_now -> tm_hour), (time_now -> tm_min), (time_now -> tm_sec));

    if ((mac != NULL) || (sn != NULL))
    {
        rc = sqlite3_open("protool.db", &db);
        if (rc != SQLITE_OK)
        {
            printf("open protool.db failure: %s\n", sqlite3_errmsg(db));
        }
        else
        {
            printf("open database protool.db success!\n");
        }

        sql_judge_table_exist = "SELECT * FROM RECORD_PROTOOL;";

        rc = sqlite3_exec(db, sql_judge_table_exist, NULL, NULL, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            printf("SQL error: %s\n", zErrMsg);
            printf("start creating TABLE 'RECORD_PROTOOL'\n");

            sql = "CREATE TABLE RECORD_PROTOOL("                           \
                   "ID      INTEGER     PRIMARY KEY     AUTOINCREMENT,"    \
                   "MAC      CHAR(32)    NOT NULL,"                        \
                   "SN       CHAR(32)    NOT NULL,"                        \
                   "TIME     CHAR(32)    NOT NULL);";                        

            rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
            if (rc != SQLITE_OK)
            {
                printf("SQL error:%s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            else
            {
                DEBUG("Table created successfully!\n");
            }
        }
        else
        {
            DEBUG("the TABLE 'RECORD_PROTOOL' have existed\n");
        }

            memset(sql_buf, 0, sizeof(sql_buf));
            snprintf(sql_buf, sizeof(sql_buf),
                    "INSERT INTO RECORD_PROTOOL(MAC,SN,TIME) VALUES('%s', '%s', '%s')", mac, sn, time_buf);

            rc = sqlite3_exec(db, sql_buf, callback, 0, &zErrMsg);
            DEBUG("sql_buf:%s\n", sql_buf);
            if (rc != SQLITE_OK)
            {
                printf("SQL error:%s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
            else
            {
                printf("insert successfully!\n");
            }
    }
}

static  int     callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int         i;

    for(i=0; i<argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i]?argv[i]:NULL);
    }
    printf("\n");

    return 0;
}

static int look_record_database(char *mac, char *sn)
{
    int         rc = -1;
    char        *zErrMsg = NULL;
    sqlite3     *db = NULL;
    char        *sql = NULL;

    rc = sqlite3_open("protool.db", &db);
    if (rc != SQLITE_OK)
    {
        printf("open protool.db failure: %s\n", sqlite3_errmsg(db));
    }
    else
    {
        printf("open database protool.db success!\n");
    }

    if(mac != NULL)
    {
        char    *sql_judge_mac = NULL;

        sql_judge_mac = "select MAC from RECORD_PROTOOL;";
        rc = sqlite3_exec(db, sql_judge_mac, print_record_mac, mac, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            DEBUG("SQL error:%s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }

    if(sn != NULL)
    {
        char    *sql_judge_sn = NULL;

        sql_judge_sn = "select SN from RECORD_PROTOOL;";
        rc = sqlite3_exec(db, sql_judge_sn, print_record_sn, sn, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            DEBUG("SQL error:%s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }
}

static int print_record_mac(void *params, int n_column, char **column_value, char **column_name)
{
    int i;
    int rv = -1;
    int flag = 0;

    //printf("\t%s\n",(char *)params);

    DEBUG("n_column = %d\n",n_column);
    for (i=0; i<n_column; i++)
    {
        //printf("\t%s\n",column_name[i]);

        if ((rv = strncmp(params, column_value[i], strlen(params))) == 0)
        {
        //    printf("\t%s\n",column_value[i]);
            flag = 1;
            break;
        }
        //printf("\t%s\n",column_value[i]);
    }
    if (flag == 1)
    {
        printf("error,the MAC was used!\n");
        printf("program exit!\n");

        exit(0);
    }

    return 0;
}

static int print_record_sn(void *params, int n_column, char **column_value, char **column_name)
{
    int i;
    int rv = -1;
    int flag = 0;

    //printf("\t%s\n",(char *)params);

    for(i=0; i<n_column; i++)
    {
      //  printf("\t%s\n",column_name[i]);

        if ((rv = strncmp(params, column_value[i], strlen(params))) == 0)
        {
        //    printf("\t%s\n",column_value[i]);
            flag = 1;
            break;
        }
        //printf("\t%s\n",column_value[i]);
    }
    if (flag == 1)
    {
        printf("error,the sn was used!\n");
        printf("program exit!\n");

        exit(0);
    }

    return 0;
}

static int parse_json_msg(char *buf, int size, int *type, char *sn_value, char *mac_value )
{
    char    *ptr_mac_value = NULL;
    char    *ptr_sn_value = NULL;
    char    *ptr_cmd_value = NULL;
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

    printf("cmd: \t%s\nsn: \t%s\nmac: \t%s\n", ptr_cmd_value, ptr_sn_value, ptr_mac_value);

    if((strncmp(ptr_cmd_value, "reply_write", strlen("reply_write"))) == 0)
    {
        *type = 1;
    }

err2:
    cJSON_Delete(sys_root);

err1:
    //free(str);
    return 0;
}
