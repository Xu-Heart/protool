/*********************************************************************************
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  eeprom.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2019年07月19日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年07月19日 16时07分56秒"
 *                 
 ********************************************************************************/

#include"eeprom_protoold.h"

int eeprom_write(char *str,int offset)
{
    int     fd = -1;
    int     rv = -1;

    if((fd  = open(EEPROM_FILE,O_RDWR,0666)) < 0)
    {
        printf("open eeprom file failure:%s\n",strerror(errno));
        return -1;
    }

    lseek(fd,offset,SEEK_SET);

    if((rv = write(fd,str,strlen(str))) < 0)
    {
        printf("write data to eeprom failure!:%s\n",strerror(errno));
        close(fd);
        return -2;
    }
    printf("write ok\n");

    close(fd);
    return 0;
}

int eeprom_read(char *buf,int byte_length,int offset)
{
    int             fd = -1;
    int             rv = -1;
    int             i;

    if((fd  = open(EEPROM_FILE,O_RDWR,0666)) < 0)
    {
        printf("open eeprom file failure:%s\n",strerror(errno));
        return -1;
    }

    lseek(fd,offset,SEEK_SET);
    memset(buf,0,sizeof(buf));

    if((rv = read(fd,buf,byte_length) < 0))
    {
        printf("read from eeprom failure!:%s\n",strerror(errno));
        close(fd);
        return -2;
    }

    if(byte_length == 11)
    {
        buf[10] = '\0';
    }
    else if(byte_length == 7)
    {
        buf[6] = '\0';
    }

    close(fd);
    return 0;
}
