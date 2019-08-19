/*********************************************************************************
 *
 *      Copyright:  (C) 2019 XuHansong<577457540@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  eeprom_interfaces.c
 *    Description:  This file offer interfaces for read or write eeprom
 *                 
 *        Version:  1.0.0(2019年08月16日)
 *         Author:  XuHansong <577457540@qq.com>
 *      ChangeLog:  1, Release initial version on "2019年08月16日 21时29分29秒"
 *                 
 ********************************************************************************/
#include "eeprom_interfaces.h"

//#define IOCTL_EEPROM

void dump_buf(char *prompt, unsigned char *data, int size)
{
    int        i;

    printf("%s\n", prompt);


    for(i=0; i<size; i++)
    {
        printf("0x%02x ", data[i]);
    }

    printf("\n");
}


/************************************************
 *  
 *    用select实现毫秒级延时 
 *    
 ************************************************/
void msleep(int msec)
{
    struct timeval     timeout;

    timeout.tv_sec=msec/1000;    //seconds
    timeout.tv_usec=msec%1000;   //micro seconds

    select(1, NULL, NULL, NULL, &timeout);
}

#ifdef IOCTL_EEPROM

/*  implement by user space I2C ioctl() driver */

#include"eeprom_interfaces.h"
int eeprom_write(unsigned char *data, int addr, int size)
{
    int     fd = -1;
    int     bytes;
    int     i = 0;
    int     compl;
    int     rv = 0;

    fd = open("/dev/i2c-0", O_RDWR);
    if(fd < 0)
    {
        printf("open file errno:%s\n", strerror(errno));
        return -1;
    }

    dump_buf("eeprom_write ioctl():", data, size);

    printf("fd(/dev/i2c-0) = %d\n", fd);

    while(size > 0)
    {
        printf("fd=%d size=%d\n", fd, size);
        compl = addr%PAGE_SIZE;

        if(compl != 0) /* 写的地址没页对齐*/
        {
            printf("write size is not page alignment\n");
            bytes = size < PAGE_SIZE ? size : PAGE_SIZE-compl;
            if((i2c_write_byte(fd, addr, &data[i], size)) < 0)
            {
                rv = -2;
                goto OUT;
            }
        }
        else        /* 写的地址有页对齐*/
        {
            printf("write size is page alignment\n");
            bytes = size < PAGE_SIZE ? size : PAGE_SIZE;
            if((i2c_write_page(fd, addr, &data[i], bytes)) < 0)
            {
                rv = -3;
                goto OUT;
            }
        }

        size -= bytes;
        i += bytes;
        addr += bytes;
    }

OUT:
    printf("write over and close fd = %d\n", fd);
    close(fd);
    return rv;
}


/******************************************************************************
 *    
 *         page_write的实现函数
 *         数据将写在页对齐处，且写的数据不大于8个字节
 *            
 ******************************************************************************/
int i2c_write_page(int fd, int addr, unsigned char *data, int bytes)
{   
    struct i2c_rdwr_ioctl_data eeprom_data;
    int     rv = 0;

    if (fd<0 || !data || bytes< 0 || addr>255 || addr<0)
    {
        printf("ERROR: %s() invalid input arguments\n", __FUNCTION__);
        return -1;
    }

    printf("%s:%d:%s() fd = %d\n", __FILE__, __LINE__, __FUNCTION__, fd);

    if ( (addr%PAGE_SIZE) != 0 )
    {
        printf("ERROR: %s() Page address is not alignment\n", __FUNCTION__);
        return -1;
    }
    if ( bytes > PAGE_SIZE)
    {
        printf("ERROR: %s() write bytes larger than page size\n", __FUNCTION__);
        return -1;
    }

    eeprom_data.nmsgs = 1;
    eeprom_data.msgs = (struct i2c_msg *)malloc(sizeof(struct i2c_msg) * eeprom_data.nmsgs);
    eeprom_data.msgs[0].len = bytes + 1;    // $addr need 1 byte 
    eeprom_data.msgs[0].addr = SLAVE_ADDR;
    eeprom_data.msgs[0].buf = malloc(eeprom_data.msgs[0].len);
    eeprom_data.msgs[0].buf[0] = addr;      // buf[0] is for addr

    memcpy(&eeprom_data.msgs[0].buf[1], data, bytes);

    printf("write %d bytes to $addr[0x%0x] fd=%d\n", bytes, addr, fd);

    dump_buf("Page write data:", eeprom_data.msgs[0].buf, (int)eeprom_data.msgs[0].len);

    if ((ioctl(fd, I2C_RDWR, &eeprom_data)) < 0)
    {
        printf("ioctl error:%s\n", strerror(errno));
        rv = -1;
        goto OUT;

    }
    else
    {
        printf("ioctl成功执行\n");
    }
    msleep(5);

OUT:
    if (eeprom_data.msgs != NULL)
    {
        free(eeprom_data.msgs);;
        eeprom_data.msgs = NULL;
    }

    return rv == 0 ? 0 : -1;
}

/**********************************************************
 *     
 *        byte write 的实现函数，调用一次该函数，
 *        i2c_write_byte在设置的偏移量处写一个字节的
 *            
 **********************************************************/
int i2c_write_byte(int fd, int addr, unsigned char *data, int bytes)
{
    struct               i2c_rdwr_ioctl_data eeprom_data;
    unsigned char        buf[2];
    int                  i = 0;
    int                  rv = 0;

    if (fd<0 || !data || bytes< 0 || addr>255 || addr<0)
    {
        printf("ERROR: %s() invalid input arguments\n", __FUNCTION__);
        return -1;
    }

    eeprom_data.nmsgs = 1;
    eeprom_data.msgs = (struct i2c_msg *)malloc(sizeof(struct i2c_msg));
    eeprom_data.msgs[0].len = 2; //offset need one byte 
    eeprom_data.msgs[0].addr = SLAVE_ADDR;
    eeprom_data.msgs[0].buf = buf;

    for (i=0; i<bytes; i++)
    {
        eeprom_data.msgs[0].buf[0] = addr+i;
        eeprom_data.msgs[0].buf[1] = data[i];

        if ((ioctl(fd, I2C_RDWR, (unsigned long) &eeprom_data)) < 0)
        {
            printf("ioctl error:%s\n", strerror(errno));
            rv = -1;
            goto OUT;
        }

        /*
         *         ioctl 将一个字节数据写到at24c02的缓冲区后，进入at24c02内部的写时序
         *         此过程一般需要5ms，在5ms以内,在次调用ioctl会导致数据无法写入缓冲区
         *         所以需要5ms的延时
         *                                    
         */
        msleep(5);
    }

OUT:
    if (eeprom_data.msgs != NULL)
    {
        free(eeprom_data.msgs);;
        eeprom_data.msgs = NULL;
    }

    return rv == 0? 0: -1;
}

int eeprom_read(unsigned char *buf, int bytes, int addr)
{
    int     fd = -1;

    fd = open("/dev/i2c-0", O_RDWR);

    if(fd < 0)
    {
        printf("open file errno:%s\n", strerror(errno));
        return -1;
    }

    if ((i2c_read_random(fd, addr, buf, bytes-1)) < 0)
    {
        close(fd);
        return -2;
    }

    buf[bytes-1] = '\0';

    close(fd);
    return 0;
}

/***********************************************************
 *  
 *     random read 的实现函数
 *     从给定的地址addr开始读指定的字节数byte的数据
 *     
 ***********************************************************/
int i2c_read_random(int fd, int addr, unsigned char *buf, int byte)
{
    struct i2c_rdwr_ioctl_data eeprom_data;
    int     rv = 0;

    if(fd<0 || !buf || byte<0 || addr>255 || addr<0)
    {
        printf("ERROR: %s() invalid input argument!\n", __FUNCTION__);
        return -1;
    }

    eeprom_data.nmsgs = 2;
    eeprom_data.msgs = (struct i2c_msg *)malloc(sizeof(struct i2c_msg) * eeprom_data.nmsgs);
    eeprom_data.msgs[0].len = 1;
    eeprom_data.msgs[0].addr = SLAVE_ADDR;
    eeprom_data.msgs[0].flags = I2C_WRITE_FLAG;
    eeprom_data.msgs[0].buf = malloc(eeprom_data.msgs[0].len);
    eeprom_data.msgs[0].buf[0] = addr;

    eeprom_data.msgs[1].len = byte;
    eeprom_data.msgs[1].addr = SLAVE_ADDR;
    eeprom_data.msgs[1].flags = I2C_READ_FLAG;
    eeprom_data.msgs[1].buf = malloc(eeprom_data.msgs[1].len);

    DEBUG("fd = %d\n", fd);
    if( (ioctl(fd, I2C_RDWR, &eeprom_data)) < 0)
    {
        printf("ioctl error:%s\n", strerror(errno));
        rv = -1;
        goto OUT;
    }

    memcpy(buf, eeprom_data.msgs[1].buf, byte);
    DEBUG("Rondom read data:%x\n", buf[0]);

OUT:
    if(eeprom_data.msgs != NULL)
    {
        free(eeprom_data.msgs);
        eeprom_data.msgs = NULL;
    }

    return rv == 0 ? 0 : -1;
}

#else

/*  implement by AT24C20 driver based on read()/write() */

#include"eeprom_interfaces.h"
int eeprom_write(unsigned char *str, int offset, int bytes)
{
    int     fd = -1;
    int     rv = -1;

    if((fd  = open(EEPROM_FILE,O_RDWR,0666)) < 0)
    {
        printf("open eeprom file failure:%s\n",strerror(errno));
        return -1;
    }

    dump_buf("Write to EEPROM Data", str, bytes);

    lseek(fd,offset,SEEK_SET);

    if((rv = write(fd, str, bytes)) < 0)
    {
        printf("write data to eeprom failure!:%s\n",strerror(errno));
        close(fd);
        return -2;
    }
    printf("write ok\n");

    close(fd);
    return 0;
}

int eeprom_read(unsigned char *buf,int byte_length,int offset)
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

    if(byte_length == 15)
    {
        buf[14] = '\0';
    }
    else if(byte_length == 7)
    {
        buf[6] = '\0';
    }

    close(fd);
    return 0;
}
#endif
