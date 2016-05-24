/*
this is the c funtion lib
fucntion:write register from bar 
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
void writeRegisters (int,int , char **);
void processArgs (int , char **);
void usage (void);
static int writeReg(int fd, unsigned reg, unsigned val);

int main(int argc, char *argv[])
{
	unsigned val;

	int fd;

	processArgs(argc, argv);

	// judge whether the device is opend and get the 
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
	writeRegisters(fd,argc, argv);

	close(fd);
	return 0;
}

/*
 * Write the register(s)
 */
void writeRegisters(int fd, int argc, char** argv)
{
	int i;
	unsigned addr;
	unsigned value;

	// Verify that we actually have some registers to display
	if (argc == 0)
	{
		usage();
		exit(1);
	}
	else if (argc % 2 == 1)
	{
		fprintf(stderr, "Error: you must supply address/value pairs\n");
		usage();
		exit(1);
	}

	// Process the registers one by one
	for (i = 2; i < argc; i += 2)
	{
		// Work out if we're dealing with decimal or hexadecimal
		if (strncmp(argv[i], "0x", 2) == 0 || strncmp(argv[i], "0X", 2) == 0)
		{
			sscanf(argv[i] + 2, "%x", &addr);
		}
		else
		{
			sscanf(argv[i], "%u", &addr);
		}

		// Work out if we're dealing with decimal or hexadecimal
		if (strncmp(argv[i + 1], "0x", 2) == 0 || strncmp(argv[i + 1], "0X", 2) == 0)
		{
			sscanf(argv[i + 1] + 2, "%x", &value);
		}
		else
		{
			sscanf(argv[i + 1], "%u", &value);
		}

		// Perform the actual register write
		writeReg(fd, addr, value);

		printf("Write: Reg 0x%08x (%u):   0x%08x (%u)\n", addr, addr, value, value);
	}
}

/*
 *  Process the arguments.
 */
void processArgs (int argc, char **argv )
{
	
}


/*
 *  Describe usage of this program.
 */
void usage (void)
{
	printf("Usage: ./regwrite <device> w [addr] [value]\n\n");
	printf("device is fpga0 or fpga1(default fpga 0)\n");
	printf("addr is the offset of the reg that you want to write\n");
    printf("value is the the data you want to write into this reg \n");
	//printf("         [addr...] is a list of one or more addresses to read\n");
}


/*
 * write a reg - write a register, using a file descriptor
 */
static int writeReg(int fd, unsigned reg, unsigned val)
{
	struct fpga_reg fpgareg;
	int ret;

	fpgareg.reg = reg;
	fpgareg.val = val;

	/* Call the ioctl */
	if ((ret = ioctl(fd, FPGA_IOCW, &fpgareg)) == 0)
	{
		return 0;
	}
	else
	{
                perror(" ioctl failed\n");
                return -1;
	}
}
