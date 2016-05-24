/*
this is some definition we use

*/

/*the definition about ioctl*/



/*
 * Ioctl definitions
 */

/* Use 'K' as magic number */
#define FPGA_IOC_MAGIC  'K'


// R means read a register
// w means write a register
// T means this is a test time 
#define FPGA_IOCT  _IO(FPGA_IOC_MAGIC,0)
#define FPGA_IOCR  _IOR(FPGA_IOC_MAGIC,  1, int )
#define FPGA_IOCW  _IOW(FPGA_IOC_MAGIC,  2, int)
#define  FPGA_IOCC _IO(FPGA_IOC_MAGIC,3)
#define FPGA_IOCSTATUS _IO(FPGA_IOC_MAGIC,4)

#define FPGA_IOC_MAXNR 5


/*
 * Structure for transferring register data via an IOCTL
 */
struct fpga_reg {
	unsigned int	reg;
	unsigned int	val;
};




