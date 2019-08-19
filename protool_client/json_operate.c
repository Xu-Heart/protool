/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  json_operate.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2019年08月18日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月18日 20时06分26秒"
 *                 
 ********************************************************************************/

#include "json_operate.h"

/******************************************************************
 *
 *      packet sn and mac as JSON format, 
 *      if success, return 0, else return negative
 *
 *****************************************************************/

int pack_json_msg(char *mac, char *sn, char *json_buf, int type)
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
    DEBUG("--------debug:strlen(json_buf) = %d\nsizeof(json_buf) = %d\n",   (int)strlen(json_buf),
            (int)sizeof(json_buf));

    return 0;

err1:
    cJSON_Delete(root);
    return -3;
}

/****************************************************************************
 *
 *         The function parse the data from server,
 *         if success, return 0,else return negative
 *
 ****************************************************************************/

int parse_json_msg(char *buf, int size, int *type, char *sn_value, char *mac_value )
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
    //free(str1);
    return 0;
}
