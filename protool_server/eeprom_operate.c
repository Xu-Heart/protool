/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  eeprom_operate.c
 *    Description:  This file is read_mac , read_sn,write_mac, write_sn function
 *                 
 *        Version:  1.0.0(2019年08月16日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月16日 15时05分05秒"
 *                 
 ********************************************************************************/

#include "eeprom_operate.h"
#include "eeprom_interfaces.h"

/****************************************************************************
 *
 *          The funstion is write mac to eeprom,
 *          if write success, return 0; else, return negtaive.
 *
 * **************************************************************************/
int write_mac(char *mac_value, int bytes)
{
    int             i = -1;
    int             rv = -1;
    char            mac_value_bake[32];
    char            *ptr = NULL;
    unsigned char   buf[6];


    memset(mac_value_bake, 0, sizeof(mac_value_bake));
    if(mac_value != NULL)
    {
        strncpy(mac_value_bake, mac_value, strlen(mac_value));
    }
    printf("mac_value_bake: %s\n", mac_value_bake);
    printf("mac_value: %s\n", mac_value);

    for (i=0; i<6; i++)
    {
        buf[i] = strtol(mac_value, &ptr, 16);
        mac_value += 3;
    }
    DEBUG("debug:%02x\n", buf[0]);

    if ((rv = eeprom_write(buf, MAC_OFFSET, HEX_MAC_BYTES)) < 0)
    {
        return -ERR_EEPROM_W;
    }
    printf("write mac to eeprom success!\n");

    return 0;
}


/************************************************************************
 *
 *          this function write sn to eeprom ,if success,return 0,
 *          else return nagative 
 *
 * **************************************************************************/
int write_sn(unsigned char *sn_value, int bytes)
{
    int     rv = -1;

    DEBUG("sn_value:%s\n", sn_value);
    if ((rv =  eeprom_write(sn_value,SN_OFFSET, bytes)) < 0)
    {
        return -ERR_EEPROM_W;
    }
    printf("write sn to eeprom success!\n");

    return 0;
}


/********************************************************************
 *
 *          The function read mac from eeprom,
 *          if success,return 0; else, return negative.
 *
 ********************************************************************/
int read_mac(unsigned char *mac, int bytes)
{
    int                  rv = -1;
    unsigned char        buf[HEX_MAC_BYTES + 1];

    memset(buf, 0, sizeof(buf));
    if ((rv = eeprom_read(buf, HEX_MAC_BYTES + 1, MAC_OFFSET)) < 0)
    {
        return -ERR_EEPROM_R;
    }

    /*  convert hex MAC to string MAC */
    snprintf(mac, bytes, "%02x:%02x:%02x:%02x:%02x:%02x",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    return 0;
}


/**********************************************************************
 *
 *          This function read SN from eeprom ,
 *          if read success, return 0; else return negative.
 *
 **********************************************************************/
int read_sn(unsigned char *sn, int sn_length)
{
    int         rv = -1;

    if ((rv = eeprom_read(sn, sn_length, SN_OFFSET)) < 0)
    {
        return -ERR_EEPROM_R;
    }
    DEBUG("read sn ok!%s\n",sn);

    return 0;
}
