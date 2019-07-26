/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  write.c
 *    Description:  This file 
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
    {"recv", required_argument, NULL, 'r'},
    {"port", required_argument, NULL, 'p'},
    {"MAC", optional_argument, NULL, 'M'},
    { "SN", optional_argument, NULL, 'S' },
    { "help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

int main(int argc, char **argv)
{
    int         ch = -1;
    int         port = -1;
    int         send = 0;
    int         recv = 0;
    int         rv = -1;
    int         pkg_rv = -1;
    int         flag = 0;
    int         rc = -1;
    char        *ptr_ip = NULL;
    const char  *optstring = "i:p:sr:M::S::h";
    char        *read_type = NULL;
    char        *ptr_mac = NULL;
    char        *ptr_sn = NULL;
    char        *str = NULL;
    char        send_buf[128];
    char        str_buf[128];
    char        json_str[128];

    printf("at main\n------------\n");
    while ((ch = getopt_long(argc, argv, optstring, opts, NULL)) != -1)
    {
        switch(ch)
        {
            case 'i' : ptr_ip = optarg; break;
            case 'p' : port = atoi(optarg); break;
            case 's' : send = 1; break;
            case 'r' : {read_type = optarg; recv = 1; }; break;
            case 'M' : ptr_mac = optarg; break; 
            case 'S' : ptr_sn = optarg; break;
            case 'h' : print_usage(argv[0]); break;
            default  : break;
        
        }
    }

    if (!ptr_ip || !port || argc <= 5)
    {
        print_usage(argv[0]);
        return -1;
    }

    if ((send==0) && (read_type==NULL))
    {
        return -1;
    }

    if ((send==1) && ((ptr_mac==NULL)&&(ptr_sn == NULL)))
    {
        printf("option '-s' must be used with option '-M' or '-S'\n");
        return -1;
    }

    /*look database record */
    look_record_database(ptr_mac,ptr_sn);


    /*pack cmd data as JSON format */
    memset(json_str, 0, sizeof(json_str));
    if ((pkg_rv = mac_sn_cJSON_pkg(ptr_mac, ptr_sn, send, recv, read_type, json_str)) < 0)
    {
        return -2;
    }

    if ((rv = connect_server(ptr_ip, port, json_str, recv, &flag)) != 0)
    {
        return -3;
    }

    /*when send MAC or SN to server,set database to save these data that have been sent by client*/
    printf("flag = %d\n", flag);

    if (flag == 1)
    {
        /*set database*/
        set_database(ptr_mac, ptr_sn);

    }

    printf("---------------\nat main()\n");
    printf("client exit\n");
    return 0;
}

void    print_usage(char *progname)
{
    printf("%s usage: \n",progname);
    printf("-M(--MAC): if you want to send  MAC to server, specify -M(--MAC)\n");
    printf("-S(--SN): if you want to send SN to server, specify -S(--SN)\n");
    printf("-s(--send): if you want to send data to server, specify -s(--send)\n");
    printf("-r(--recv): if you want to receive data from server, specify -r(--recv)\n");
    printf("-p(--port): sepcify the port that client will to connect server's port\n");
    printf("-i(--ipaddr): sepcify the ipaddr that client will to connect server's ipaddress\n");
    printf("-h(--help): print help information\n");

    return ;
}

/*create sockfd to connect server and net io */
int connect_server(char *ip, int port, char *str, int recv, int *flag)
{
    int                     rv = -1;
    int                     sockfd = -1;
    struct sockaddr_in      servaddr;
    int                     send_rv = -1;
    int                     recv_rv = -1;
    char                    recv_buf[128];

    printf("------------------\nat connect_server()");

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
        printf("%d\n", rv);
        printf("connect to server [%s:%d] error:%s\n", ip, port, strerror(errno));
        close(sockfd);
        return -5;
    }
    else
    {
        printf("connect to server[%s:%d] successfully\n", ip, port);
        printf("str = %s\n", str);

        if ((send_rv = write(sockfd, str, strlen(str))) <= 0)
        {
            printf("write data to server failure:%s\n", strerror(errno));

            close(sockfd);
            return -3;
        }
        printf("send data to server success!\n");

        /*if send MAC or SN to server success,set flag = 1*/
        if ((recv==0) && (send_rv>0))
        {
            *flag = 1;
        }
        
        if (recv == 1)
        {
            memset(recv_buf, 0, sizeof(recv_buf));
            printf("at recv\n");

            if ((recv_rv=read(sockfd, recv_buf, sizeof(recv_buf))) <= 0)
            {
                printf("recv data from server failure:%s\n", strerror(errno));
                return -4;
            }
            printf("recv %d byte data from server success:%s\n", recv_rv, recv_buf);

        }

    }

    return 0;
}

/* package mac an sn as JSON format*/
int mac_sn_cJSON_pkg(char *ptr_mac, char *ptr_sn, int send, int recv, char *read_type,char *json_pkg)
{
    char    *ptr_json = NULL;
    int     rv = -1;

    printf("-------------------\nat mac_sn_cJSON_pkg\n");

    cJSON *root = cJSON_CreateObject();

    if (NULL == root)
    {
        return -1;
    }

    printf("send = %d, recv = %d\n", send, recv);
    
    if (send == 1)
    {
        cJSON_AddStringToObject(root, "cmd", "write");
    }
    if (recv == 1)
    {
        cJSON_AddStringToObject(root, "cmd", "read");
    }

    cJSON *data = cJSON_CreateObject();
    if (NULL == data)
    {
        goto err1;
    }

    if (ptr_mac != NULL)
    {
        printf("ptr_mac != NULL\n");
        cJSON_AddStringToObject(data, "mac", (const char *)ptr_mac);
    }
    
    if (ptr_sn != NULL )
    {
        printf("ptr_sn != NULL\n");
        cJSON_AddStringToObject(data, "sn", (const char *)ptr_sn);
    }

    if (recv == 1)
    {

        if ((rv=strcmp(read_type,"mac")) == 0)
        {
            printf("----------------------debug:at read_type mac\n");
            cJSON_AddStringToObject(data, "read_type", read_type);
        }
        if ((rv=strcmp(read_type,"sn")) == 0)
        {
            printf("----------------------debug:at read_type sn\n");
            cJSON_AddStringToObject(data, "read_type", read_type);
        }

        /*  
        if((rv = strncmp(read_type,"mac_sn",strlen("mac_sn"))) == 0)
        {
            printf("----------------------debug:at read_type mac_sn\n");
            cJSON_AddStringToObject(data,"read_type",read_type);
        }
        */
    }

    cJSON_AddItemToObject(root, "data", data);
    ptr_json = cJSON_PrintUnformatted(root);

    printf("strlen(ptr_json) = %d\n", (int)strlen(ptr_json));
    strncpy(json_pkg, ptr_json, strlen(ptr_json));
    printf("--------debug:ptr_json:%s\n", ptr_json);
    printf("--------debug:json_pkg:%s\n", json_pkg);
    printf("--------debug:strlen(json_pkg) = %d\nsizeof(json_pkg) = %d\n",     
          (int)strlen(json_pkg), (int)sizeof(json_pkg));

    return 0;

err1:
    cJSON_Delete(root);
    return -3;
}

int     set_database(char *ptr_mac, char *ptr_sn)
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


    printf("--------------------\nat set_database\n");

    /*get systime */
    time(&timep);
    time_now = localtime(&timep);

    memset(time_buf, 0, sizeof(time_buf));
    snprintf(time_buf, sizeof(time_buf), "%d-%d-%d %d:%d:%d", (time_now -> tm_year + 1900), (time_now -> tm_mon + 1), (time_now -> tm_mday), (time_now -> tm_hour), (time_now -> tm_min), (time_now -> tm_sec));

    if ((ptr_mac != NULL) || (ptr_sn != NULL))
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
                printf("Table created successfully!\n");
            }
        }
        else
        {
            printf("the TABLE 'RECORD_PROTOOL' have existed\n");
        }

            memset(sql_buf, 0, sizeof(sql_buf));
            snprintf(sql_buf, sizeof(sql_buf),
                    "INSERT INTO RECORD_PROTOOL(MAC,SN,TIME) VALUES('%s', '%s', '%s')", 
                    ptr_mac, ptr_sn, time_buf);

            rc = sqlite3_exec(db, sql_buf, callback, 0, &zErrMsg);
            printf("sql_buf:%s\n", sql_buf);
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

int     look_record_database(char *ptr_mac, char *ptr_sn)
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

    if(ptr_mac != NULL)
    {
        char    *sql_judge_mac = NULL;

        sql_judge_mac = "select MAC from RECORD_PROTOOL;";
        rc = sqlite3_exec(db, sql_judge_mac, print_record_mac, ptr_mac, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            printf("SQL error:%s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }

    if(ptr_sn != NULL)
    {
        char    *sql_judge_sn = NULL;

        sql_judge_sn = "select SN from RECORD_PROTOOL;";
        rc = sqlite3_exec(db, sql_judge_sn, print_record_sn, ptr_sn, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            printf("SQL error:%s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }
}

int print_record_mac(void *params, int n_column, char **column_value, char **column_name)
{
    int i;
    int rv = -1;
    int flag = 0;

    //printf("\t%s\n",(char *)params);

    printf("n_column = %d\n",n_column);
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

int print_record_sn(void *params, int n_column, char **column_value, char **column_name)
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
