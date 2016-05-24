/*
this is the c funtion lib
fucntion:read register from bar 
auth:zj
date:2015/06/02
version: 1.0 
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include <sys/ioctl.h>
#include<sys/types.h>
#include<fcntl.h>



/* Function declarations */
void readRegisters (int,int , char **);
void processArgs (int , char **);
void usage (void);
int readReg(int fd, unsigned reg, unsigned *val);


/*now we */
int main(int argc, char *argv[])
{
	unsigned val;
    int fd;

	processArgs(argc, argv);

  /*
    if( (strcmp(argv[1],"fpga0")==0)){
        fd=open("/dev/FPGA_for_bigdata",O_RDONLY);
        printf("open the fpga0");
    }else if(0==strcmp(argv[1],"fpga1")){
        fd=open("/dev/map_fpga2",O_RDONLY);
        printf("open the device map_fpga2");
        printf("fpga1 \n");
        ;
    }else
        perror("error device(not fpga0 or fpga1)\n");
    */
    
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
	// Read the registers
	readRegisters(fd, argc, argv);

	close(fd);

	return 0;
}



/*
 * Read the register(s)
 */
void readRegisters(int fd, int argc, char**argv)
{
	int i;
	unsigned addr;
	unsigned value;

	// Verify that we actually have some registers to display
	if (argc == 0) {
		usage();
		exit(1);
	}

	// Process the registers one by one
	for (i = 2; i < argc; i++)
	{
		// Work out if we're dealing with decimal or hexadecimal
		if (strncmp(argv[i], "0x", 2) == 0 || strncmp(argv[i], "0X", 2) == 0)
		{
			sscanf(argv[i]+2, "%x", &addr);
		}
		else
		{
			sscanf(argv[i], "%u", &addr);
		}
        printf("the addr is %x\n",addr);
        printf("the fd is %d\n",fd);
		// Perform the actual register read
		readReg(fd, addr, &value);

		printf("Reg 0x%08x (%u):   0x%08x (%u)\n", addr, addr, value, value);
	}
    
}

/*
 *  Process the arguments.
 *  
 */
void processArgs (int argc, char **argv )
{
	

	/* optind starts at first non-option argument (register address) */

	if (argv[optind] == NULL) {
		usage(); exit(1);
	}
}

/*
 *  Describe usage of this program.
 */
void usage (void)
{
	printf("Usage: ./regread <device> r [addr] \n\n");
	printf("device is fpga0 or fpga1(default fpga 0)\n");
	printf("         [addr...] is a list of one or more addresses to read\n");
}


/*
 * readReg - read a register
 */
int readReg(int fd, unsigned reg, unsigned *val)
{
	
	struct fpga_reg fpgareg;
	int ret=5;

	fpgareg.reg = reg;

    
	/* Call the ioctl */
   ret=ioctl(fd, FPGA_IOCR, &fpgareg);
   printf("the ret is %d\n",ret);
	if (ret == 0)
	{
		*val = fpgareg.val;
		return 0;
	}
	else
	{
                perror(" ioctl failed\n");
                return -1;
	}

}