/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  json.c
 *    Description:  This file parse json format data or packet data as json
 *                 
 *        Version:  1.0.0(2019年08月16日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月16日 14时44分51秒"
 *                 
 ********************************************************************************/


/********************************************************
 *  
 *     parse JSON format data from client
 *     if success ,return 0,else,return negative 
 *     
 ********************************************************/

#include "json_operate.h"

int parse_json_msg(char *buf, int size, int *type, char *sn_value, char *mac_value )
{
    char    *ptr_mac_value = NULL;
    char    *ptr_sn_value = NULL;
    char    *ptr_cmd_value = NULL;

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

    printf("cmd = %s, sn = %s, mac = %s\n", ptr_cmd_value, ptr_sn_value, ptr_mac_value);

    /* justice cmd is "write" or "read"*/
    if (!(strncmp(ptr_cmd_value, "write", strlen("write"))))
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

    /* when cmd is read,read mac and sn from eeprom*/
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

int pack_json_msg(char *mac_buf, char *sn_buf, char *json, int type)
{
    char    *json_pkg = NULL;

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

    DEBUG("%s\n", mac_buf);
    DEBUG("%x\n", (unsigned int)(*mac_buf));
    DEBUG("%s\n", sn_buf);
    DEBUG("%x\n", (unsigned int)(*sn_buf));

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

    DEBUG("json_pkg: %s\n", json_pkg);
    DEBUG("strlen(json_pkg) = %d\n", (int)strlen(json_pkg));
    printf("reply client json: %s\n", json);

    return 0;

err1:
    cJSON_Delete(root);
    return -3;
}

