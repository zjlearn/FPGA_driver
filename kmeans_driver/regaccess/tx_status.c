#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include <sys/ioctl.h>
#include<sys/types.h>
#include<fcntl.h>



int main(int argc, char *argv[])
{
    int fd;
	char devicename[30];
    strcpy(devicename,"/dev/");
    strcat(devicename,argv[1]);
    puts(devicename);
    fd=open(devicename,O_RDONLY);
    
    
    if(fd <0 ){
        printf("failed to open the device \n");
    }else{
        printf("open the device scuess!\n");
    }
    ioctl(fd, FPGA_IOCSTATUS);
}
