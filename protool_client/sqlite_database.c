/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  sqlite_database.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2019年08月18日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月18日 20时17分00秒"
 *                 
 ********************************************************************************/

#include "sqlite_database.h"

int set_database(char *mac, char *sn)
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

    /* get systime */
    time(&timep);
    time_now = localtime(&timep);

    memset(time_buf, 0, sizeof(time_buf));
    snprintf(time_buf, sizeof(time_buf), "%d-%d-%d %d:%d:%d", (time_now -> tm_year + 1900), (time_now ->
        tm_mon + 1), (time_now -> tm_mday), (time_now -> tm_hour), (time_now -> tm_min), (time_now -> tm_sec));

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

int     callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int         i;

    for(i=0; i<argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i]?argv[i]:NULL);
    }
    printf("\n");

    return 0;
}

int look_record_database(char *mac, char *sn)
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

int print_record_mac(void *params, int n_column, char **column_value, char **column_name)
{   
    int i;      
    int rv = -1;
    int flag = 0;

    DEBUG("n_column = %d\n",n_column);

    for (i=0; i<n_column; i++)
    {
        if ((rv = strncmp(params, column_value[i], strlen(params))) == 0)
        {
            flag = 1;
            break;
        }      
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

    for(i=0; i<n_column; i++)
    {
        if ((rv = strncmp(params, column_value[i], strlen(params))) == 0)
        { 
            flag = 1;
            break;
        }
    }

    if (flag == 1)
    {
        printf("error,the sn was used!\n");
        printf("program exit!\n");

        exit(0);
    }

    return 0;
}
