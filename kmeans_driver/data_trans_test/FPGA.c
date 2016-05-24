//
// Created by zj on 2015/4/23.
//this is a pcie driver
//




#include <linux/interrupt.h>   //for interrupt
#include<linux/pci.h>
#include<asm/io.h>   /*for ioremap function*/
#include<linux/device.h>
#include<linux/ioport.h>  /*for request_mem_region*/
#include<linux/dma-mapping.h>
#include<asm/io.h>
#include "FPGA.h"
#include "buffer.h"

#include "./regaccess/common.h"



//Fill in kernel structure with a list of ids this driver can handle
//and the driver can handle two device
MODULE_LICENSE("GPL");
static struct pci_device_id pcie_fpga_ids[]={
        {PCI_DEVICE( VENDER_ID_ONE,DEVICE_ID_ONE)},	
        {PCI_DEVICE(VENDER_ID_TWO,DEVICE_ID_TWO)},
        {0,}
};



 //for test
int map_num; 
int reduce_num;

//some declaration about the following function
static int probe(struct pci_dev *pdev, const struct pci_device_id *id);
static void remove(struct pci_dev *dev);


int fpga_open(struct inode *inode, struct file *filePtr);
int fpga_close(struct inode *inode, struct file *filePtr);
ssize_t fpga_read(struct file *filePtr, char __user *buf, size_t count, loff_t *pos);
ssize_t fpga_write(struct file *filePtr, const char __user *buf, size_t count, loff_t *pos);
int fpga_ioctl (struct inode *inode, struct file *filp,
                unsigned int cmd, unsigned long arg);


                
//the func relate to the sork_struct


//this is the 
MODULE_DEVICE_TABLE(pci,pcie_fpga_ids);



static struct pci_driver fpgaDriver = {
        .name = DRIVER_NAME,
        .id_table = pcie_fpga_ids,
        .probe = probe,
        .remove = remove,
};
//file operation structure .
static struct file_operations fileOps = {
        .owner =    THIS_MODULE,
        .read =     fpga_read,
        .write =    fpga_write,
        .open =     fpga_open,
        .release =  fpga_close,
        .ioctl = fpga_ioctl,

};




/**the interrupt handle function  do the following work
 *the interrupt may occur when read and write
 * read:set DMA addr ,len ,enable DMA and then change the DMA ptr when the tranmission is finishing
 *
 * write:change the DMA ptr
 */
/*
static irqreturn_t fpga_interrupt(int irq, void *dev_id) //delete the parameter of  struct pt_regs *regs
{
    //disable irq, i am not sure this staement, i write this because the machine is reboot

    struct fpga_device *fpga;
    
    u32 status;
    
    fpga=(struct fpga_device *)dev_id;   //modify in this version 2015/06/15
  
   
    
    //read the interrupt status
    status= ioread32((void *)fpga->baseAddr + INTERRUPTSTATUS_REG);  
    printk(KERN_INFO "the value  of  status reg is %d", status);
    //judge the value of error
    if(status==2) {
        
            printk(KERN_INFO
            "the interrupt because of DMA read_complete\n");
            //update the tx_buffer status and the list point.
               if (fpga->workflag== 1)
                   fpga->ready=1;
             //wake up the write process
               wake_up_interruptible(&fpga->inq);
             return IRQ_HANDLED;
        }else if(status ==1){
            printk(KERN_INFO
            "the interrupt because of DMA write_complete\n ");
            //update the ready
               if(fpga->workflag==2)
                    fpga->ready=1;
            
            //update the buffer state
           struct rx_buffer_struct * result=list_entry(fpga->rxAssign,struct rx_buffer_struct,list);
           result->status=1; 
           
           //update the point rxAssign 
           fpga->rxAssign=fpga->rxAssign->next;
           struct rx_buffer_struct * nextresult= list_entry(fpga->rxAssign,struct rx_buffer_struct,list);
           //update the WRITEADDRLOW_REG(write the next result's address into the WRITEADDRLOW_REG)
           iowrite32(nextresult->dma_bus_rx, fpga->baseAddr + WRITEADDRLOW_REG); 
           
           //wake up the read function 
           wake_up_interruptible(&fpga->outq);
            return IRQ_HANDLED;
        }else{
            printk(KERN_INFO "interrupt Error\n");
            //this 
            return IRQ_HANDLED;
        }
       
}
*/

/*
 * The ioctl() implementation
 */

int fpga_ioctl (struct inode *inode, struct file *filp,
                unsigned int cmd, unsigned long arg)
{
   
    int err = 0, ret = 0;

    struct fpga_device *fpga;

    fpga = filp->private_data;

    struct fpga_reg reg;
    printk(KERN_INFO "one\n");
    /* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
    if (_IOC_TYPE(cmd) != FPGA_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > FPGA_IOC_MAXNR) return -ENOTTY;
    printk(KERN_INFO "two\n");
    /*
     * the type is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. Note that the type is user-oriented, while
     * verify_area is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err)
        return -EFAULT;

    printk(KERN_INFO "threee\n");

    switch(cmd) {
        case FPGA_IOCT:  /*this is a test for ioctl*/
            printk(KERN_INFO "this is a test for ioctl\n");
            break;

        case FPGA_IOCR: /*read a reg*/
            printk(KERN_INFO "read the reg\n");
            if (copy_from_user(&reg, (void *)arg, sizeof(struct fpga_reg))) {
                printk(KERN_ERR "fpga: Unable to copy data from user space\n");
                return -EFAULT;
            }
            printk(KERN_INFO "reg is %x \n" ,reg.reg);
            printk(KERN_INFO "the value of all address is %x\n",fpga->baseAddr + reg.reg);
            reg.val = ioread32(fpga->baseAddr + reg.reg);
            printk(KERN_INFO "the reg value is %x\n", reg.val);
            if (copy_to_user((void *)arg, &reg, sizeof(struct fpga_reg))) {
                printk(KERN_ERR "fpga: Unable to copy data to user space\n");
                return -EFAULT;
            }
            ret=0;
            break;

        case FPGA_IOCW: /*write a reg*/
            printk(KERN_INFO "write the reg\n");
            if (copy_from_user(&reg, (void *)arg, sizeof(struct fpga_reg))) {
                printk(KERN_ERR "fpga: Unable to copy data from user space\n");
                return -EFAULT;
            }
            iowrite32(reg.val, fpga->baseAddr + reg.reg);
            printk(KERN_INFO "the value of reg is %x\n", reg);
            printk(KERN_INFO "the value of all address is %x\n",fpga->baseAddr + reg.reg);
            printk(KERN_INFO "the value to write is %x\n",reg.val);
            printk(KERN_INFO "after the write reg\n");

            break;
        default:  /* redundant, as cmd was checked against MAXNR */
            return -ENOTTY;
    }
    return ret;
}
/**
 * open function do the following work
 * set the fpga
 * register the irq
 */

int fpga_open(struct inode *inode, struct file *filePtr) {
     //Get a handle to our devInfo and store it in the file handle
    struct fpga_device * fpga;
	int err;
    printk(KERN_INFO "fpga_open: Entering function.\n");
   
    fpga = container_of(inode->i_cdev,struct fpga_device,cdev);
    filePtr->private_data = fpga;

    if (down_interruptible(&fpga->sem)) {
        printk(KERN_WARNING " fpga_open: Unable to get semaphore!\n");
        return -1;
    }else{
        printk(KERN_INFO "scuess get the semaphore!\n");
    }
    //fpga->result=0; //init the var of result
    //Return semaphore
    up(&fpga->sem);

    if (down_interruptible(&fpga->sem)) {
        printk(KERN_WARNING "[BBN FPGA] fpga_open: Unable to get semaphore!\n");
        return -1;
    }
    /*
    
    printk(KERN_INFO "get the irq info\n");

    printk(KERN_INFO "the irq is %d\n",fpga->pdev->irq);
    
    if( fpga->opencount==0){
         err = request_irq(fpga->pdev->irq, fpga_interrupt, IRQF_DISABLED,DEVICE_NAME, fpga);
        if (err) {
            printk(KERN_ERR "fpga: Unable to allocate interrupt "
                    "handler: %d\n", err);
        }else{
            printk(KERN_INFO "request_irq scuessfully!\n");
        }
    }*/
    fpga->opencount++;
    up(&fpga->sem);
    
    //whether we need enable the irq using write the register
    printk(KERN_INFO " fpga_open: Leaving function.\n");

    return 0;
}

/**close function do the following work
 *
 *
 */
int fpga_close(struct inode *inode, struct file *filePtr){
     struct fpga_device * fpga;
    fpga= (struct fpga_device *)filePtr->private_data;

    if (down_interruptible(&fpga->sem)) {
        printk(KERN_WARNING "fpga_close: Unable to get semaphore!\n");
        
        return -1;
    }
    fpga->opencount--;
    //diable and release the irq
    /*
    if(fpga->opencount==0)
        free_irq(fpga->pdev->irq,fpga);
    */
    //TODO: some checking of who is closing.
    up(&fpga->sem);
    printk(KERN_INFO ";leave the fpga_close function\n");
    return 0;
}


/**
the function is to send the task into the device and relate result with the rx  buffer. 
*/
void sendTask(struct  fpga_device * fpga){
    
    //struct fpga_device * fpga=(struct fpga_device *)device;
    printk(KERN_INFO "enter the send task function! ******\n");
    
    //int ready= ioread32((void *)fpga->baseAddr + READY_REG);
    //this is a test. the main boject is to read data from the rx_buffer
    struct rx_buffer_struct * test_rx=list_entry(fpga->rxAssign,struct rx_buffer_struct,list);
    unsigned char  * p=NULL;
    p=test_rx->dma_buffer_rx;
    int i;
    for (i=0 ; i<64; i++)
        if( (*(p+i)) != 0)
            printk(KERN_EMERG "%x ", *(p+i));
    //this is a test, it will be deleted in the future
    u32 status= ioread32((void *)fpga->baseAddr + INTERRUPTSTATUS_REG);
    
    if(status ==2 ) //read finished
    {
        fpga->read_finished=1;
        printk(KERN_EMERG "read finished!~~~~~~~~~~~~\n");
        wake_up_interruptible(&fpga->inq);
    }
    else if(status ==1){
        fpga->write_finished=1;
        fpga->read_finished=1;
        struct rx_buffer_struct * result=list_entry(fpga->rxAssign,struct rx_buffer_struct,list);
           result->status=1; 
           printk(KERN_EMERG "write_finished ! ~~~~~~~~~~\n");
           //update the point rxAssign 
           fpga->rxAssign=fpga->rxAssign->next;
             wake_up_interruptible(&fpga->outq);
        
    }
    if( !fpga->read_finished && !fpga->write_finished)
        return;
    if(down_trylock(& (fpga->tx_sem)))  //we only try to lock the sem
        return ;
    if(down_trylock(& (fpga->rx_sem)))
        return;
        
    printk(KERN_INFO "get the buffer lock !\n");
    

    //judge whether there some is data to send 
    struct tx_buffer_struct * task=find_tx_buffer(fpga,1);
    if(task == NULL)
    {
        printk(KERN_INFO "the task buffer is empty!");
        goto out;
    }
    //find whether there is some space to 
    struct rx_buffer_struct * result_buffer = find_rx_buffer(fpga ,0);
    if(result_buffer == NULL){
        printk(KERN_INFO  "***************get the rx_buffer is empty !\n");
    goto out;}
    
    //update the ready 
    fpga->read_finished=0;
    fpga->write_finished=0;
    
   printk(KERN_INFO "the task is to send in the %d tx_buffer!\n", task->num);
   printk(KERN_INFO "the result is to sent into %d rx_buffer !\n",result_buffer->num);
   
    fpga->write_num=fpga->write_num + 1;
    printk(KERN_INFO "has send the task %d times\n",fpga->write_num);
    
    //send the data and update the tx_buffer and rx_buffer
    iowrite32(task->dma_bus_tx , fpga->baseAddr + READADDRLOW_REG);
    u64 high_tx_address=(task->dma_bus_tx )>>32;
    printk(KERN_INFO "the high address of tx_buffer is %lx\n",high_tx_address);
    iowrite32(high_tx_address , fpga->baseAddr + READADDRHIGH_REG);
    //iowrite32(0x17 , fpga->baseAddr + READADDRHIGH_REG);
    iowrite32(task->len, fpga->baseAddr + READDATALEN_REG);
    wmb(); //to ensure the operation of write not to change the order
    iowrite32(START_DMA,fpga->baseAddr+DMA_ENABLE_REG);
    printk(KERN_INFO "the dma_bus_tx is %x", task->dma_bus_tx);
    //update the tx and rx buffer status
    task->status=0;
    result_buffer->status= -1;  //meaning this buffer has been assigned;
    result_buffer->pid=task->pid;
    
    //write the result address into the reg 
    iowrite32(result_buffer->dma_bus_rx, fpga->baseAddr + WRITEADDRLOW_REG);
    printk(KERN_INFO "the address of rx is %lx",result_buffer->dma_bus_rx );
    u64 high_rx_address=result_buffer->dma_bus_rx>>32;
    printk(KERN_INFO "the high address of rx is %lx",high_rx_address);
    iowrite32(high_rx_address, fpga->baseAddr + WRITEADDRHIGH_REG);
    
    //update the list point
    fpga->txNextFull=fpga->txNextFull->next;
    //temp=list_entry(fpga->txNextFull,struct tx_buffer_struct,list);
    //printk(KERN_INFO "the full point is %d tx_buffer ！\n", temp->num);
    
    fpga->rxUnassign=fpga->rxUnassign->next;
    
    up(&(fpga->tx_sem));
    up(&(fpga->rx_sem));
   
    
    printk(KERN_INFO "leave the sendTask function \n");
    return ;
    
 out:  //send  failed。 should release the semaphore we alloc . 
    printk(KERN_INFO "error in the sendTask function !\n");
    up(&(fpga->tx_sem));
    up(&(fpga->rx_sem));
    return 0;
    
}





//the timer handler function
void timerHandler(unsigned long data){
    struct fpga_device * fpga=(struct fpga_device *)data;
    sendTask(fpga);  //send the task data into the FPGA 
       
    printk(KERN_INFO "sendTask timer! \n");// this is a test for tier
    //we add the sendtask task into the tasklet to enhance the speed
    
    //modify the expire and and the timer into the kernel again
    fpga->myTimer.expires=jiffies+TIME_DELAY*HZ;
    add_timer(&fpga->myTimer);
}




//Pass-through to main dispatcher
/**the read  function do the following work
 * 
 * 
 */
ssize_t fpga_read(struct file *filePtr, char __user *buf, size_t count, loff_t *pos){

   
    struct fpga_device *fpga=filePtr->private_data;

    rxlist  rx_buffer;
    
    printk(KERN_EMERG "the process %d is in the fpga_read function --------\n", current->tgid);
    printk(KERN_INFO "the current process is %s .",current->comm);
    //I am not sure whether it is right to get the sem like that 
    if(down_interruptible(&fpga->rx_sem))
        return ;//return -ERESTARTSYS;
    while(find_rx_buffer(fpga,1)== NULL ){  //没有本进程的数据可以读
        up(&fpga->rx_sem);
        printk(KERN_INFO " \" %s\" reading : going to sleep\n", current->comm);
        test(fpga);
        if(wait_event_interruptible(fpga->outq,find_rx_buffer(fpga,1)!=NULL))
            return;//return -ERESTARTSYS; //信号，通知fs层做相应的处理
        //否则循环，但首先获得锁
        if(down_interruptible(&fpga->rx_sem))
            return ;//return -ERESTARTSYS;
    }
    
    
    rx_buffer=find_rx_buffer(fpga,1);
    //the data is ready,
    //read from the data from the buffer 
    if(copy_to_user(buf,rx_buffer->dma_buffer_rx,count)){ //copy the data to the user directly
        up(&fpga->rx_sem);
        printk(KERN_INFO "the copy_to_user is sothing wrong！\n");
        return -EFAULT;
    }
    printk(KERN_INFO "fpga_read get the result from the %d rx_buffer!\n",rx_buffer->num);
    
    //update the status info       
    rx_buffer->status=0;     
    //update the point
    fpga->rxFull=fpga->rxFull->next;
    
    fpga->current_result_pid= -1;  //in case there is the same pid to read result;
    //考虑是否可以解决上面的困境，可否可以设置cureent_result_pid=rxFul->pid;
    
    //some unlock, this should be modified in the later
    up(&fpga->rx_sem);
    
    return count;
     
}



/**
*this is get the write space 
*this function deal with the block and the sem 
*/
static int  getwritespace(struct fpga_device * fpga){
    printk(KERN_INFO "enter the getwritespace function \n");
    while(find_tx_buffer(fpga,0)==NULL){ //full
        DEFINE_WAIT(wait);
        up(&fpga->tx_sem);
        
        printk(KERN_INFO "\" %s \" the writing is going to sleep \n",current->comm);
        prepare_to_wait_exclusive(&fpga->inq,&wait,TASK_INTERRUPTIBLE);
        if(find_tx_buffer(fpga,0)==NULL)
            schedule();
        if(signal_pending(current))
        {
            return -ERESTARTSYS;
        }
            
        if(down_interruptible(&fpga->tx_sem)){
            return -ERESTARTSYS;
        }
            
    }
    printk(KERN_INFO "leave the getwritespace function \n");
    return 0;
}


/**the write function do the following work
 * get a buffer to store the data 
 *
 */
 
ssize_t fpga_write(struct file *filePtr, const char __user *buf, size_t count, loff_t *pos){
   
    int result;
     printk(KERN_EMERG "the process %d is in the fpga_write function+++++++++", current->tgid);
    struct fpga_device * fpga =filePtr->private_data; 
    
    //version 1
    /*
    u64 virt_addr= __get_free_pages(GFP_KERNEL,10);
    if( ! virt_addr){
            printk(KERN_INFO "alloc the buffer failed! \n");
            return 0;
    }
    u64 bus_addr= virt_to_bus(virt_addr);
    int notcopy=copy_from_user(virt_addr,buf,count);
    printk(KERN_INFO "the notcopy is %d\n",notcopy);
     //send the data and update the tx_buffer and rx_buffer
    printk(KERN_EMERG "the address of data  is %lx\n",bus_addr);
    iowrite32(bus_addr , fpga->baseAddr + READADDRLOW_REG);
    u32 high_tx_address=(bus_addr )>>32;
    printk(KERN_EMERG "the high address of tx_buffer is %lx\n",high_tx_address);
    iowrite32(high_tx_address , fpga->baseAddr + READADDRHIGH_REG);
    iowrite32(count, fpga->baseAddr + READDATALEN_REG);
    wmb(); //to ensure the operation of write not to change the order
    iowrite32(START_DMA,fpga->baseAddr+DMA_ENABLE_REG);
    printk(KERN_INFO "strart the DMA transport!\n");
   */
   /*
   //version 2 dynamic map
   //by this way , we can get data from the RAM to the FPGA 
    void * virt_addr= __get_free_pages(__GFP_HIGHMEM,4);
    if( ! virt_addr){
            printk(KERN_EMERG "alloc the buffer failed! \n");
            return 0;
    }
    dma_addr_t bus_addr=dma_map_single( &fpga->pdev->dev, virt_addr, 16*PAGE_SIZE, DMA_TO_DEVICE);
    if(bus_addr == NULL){
        printk(KERN_EMERG "map the buffer failed! \n");
    }
    printk(KERN_EMERG "the  bus  addr is  %lx \n" , bus_addr);
    int notcopy=copy_from_user(virt_addr,buf,count);
    printk(KERN_INFO "the notcopy is %d\n",notcopy);
     //send the data and update the tx_buffer and rx_buffer
    iowrite32(bus_addr , fpga->baseAddr + READADDRLOW_REG);
    u32 high_tx_address=(bus_addr )>>32;
    printk(KERN_EMERG "the high address of tx_buffer is %lx\n",high_tx_address);
    iowrite32(high_tx_address , fpga->baseAddr + READADDRHIGH_REG);
    iowrite32(count, fpga->baseAddr + READDATALEN_REG);
    wmb(); //to ensure the operation of write not to change the order
    iowrite32(START_DMA,fpga->baseAddr+DMA_ENABLE_REG);
    printk(KERN_INFO "strart the DMA transport!\n");
    
    */
    
    //version 3
    
    txlist  tx_buffer;
    
    //get the lock
    if( down_interruptible(&fpga->tx_sem))
    {
        return -ERESTARTSYS;
    }
    result =getwritespace(fpga);
    if(result){  
        printk(KERN_INFO "the error \n");  //
        return 0;   //getwritespace has up(&fpga->tx_sem);
    }
    printk(KERN_INFO "the current processs is %s",current->comm);
    tx_buffer=find_tx_buffer(fpga,0);
     //printk(KERN_INFO　"the buffer is towrite is %d tx_buffer!\n", tx_buffer->num);    //copy the data into the buffer
    //copy_from_user(to, from ,len)
    int notcopy=copy_from_user(tx_buffer->dma_buffer_tx,buf,count);//copy the data to the user directly
   // out(buf,count);
    tx_buffer->status=1;  //update the status of buffer
    tx_buffer->len=count;
    printk(KERN_INFO "the size of data is to write is %d \n", count);
    printk("the virt address of data is %lx\n", tx_buffer->dma_buffer_tx);
    printk("the bus address of data is %lx\n", tx_buffer->dma_bus_tx);
    printk("the high address of data is %lx\n", tx_buffer->dma_bus_tx>>32);
    printk("the low address of data is %lx\n", tx_buffer->dma_bus_tx);
    printk("the size of dma_addr_t is  %d\n", sizeof(dma_addr_t));
    //tx_buffer->pid=current->pid;
    tx_buffer->pid=current->tgid;
    //update the status
    fpga->txNextEmpty= fpga->txNextEmpty->next;
    
    up(&fpga->tx_sem);
    
    //for test 
    unsigned char  * p=NULL;
    p=tx_buffer->dma_buffer_tx;
    printk(KERN_INFO "the first number of data is %x %x %x %x\n", *p, *(p+1),*(p+17),*(p+28));
    printk(KERN_INFO "leave the fpga_write fucntion \n");

    sendTask(fpga);
    
    return count-notcopy;
}

//setup the char device
static int setup_chrdev(struct fpga_device *fpga,int flag){
    /*
    Setup the /dev/deviceName to allow user programs to read/write to the driver.
    */
     printk(KERN_INFO "Enter the setup_chrdev function\n");
          
    int devMinor =0 ;
   
    int devMajor =101+map_num+reduce_num;
	fpga->cdevNum=MKDEV(devMajor,devMinor);
    int result = register_chrdev_region(fpga->cdevNum, 1, BOARD_NAME); 
    if (result < 0) {
        printk(KERN_WARNING  "Can't get major ID\n");
        return -1;
    }
	/*set up the char dev */

    //Initialize and fill out the char device structure
    //connect the file operation with the cdev
    cdev_init(&fpga->cdev, &fileOps);
    fpga->cdev.owner = THIS_MODULE;
    fpga->cdev.ops = &fileOps;
    /*connect the major /minor number to the cdev*/
    result = cdev_add(&fpga->cdev, fpga->cdevNum, 1);  /*dev represent the major and minor number*/
    if (result) {
        printk(KERN_NOTICE "Error %d adding char device for  FPGA driver with major/minor %d / %d", result, devMajor, devMinor);
        return -1;
    }
    /*other statement*/
    char * device_name;
    if(flag==1){
        if(1==map_num)
            device_name="mapfpga1";
        else if(2==map_num)
            device_name=="mapfpga2";
        else if(3==map_num )
            device_name="mapfpga3";
        else 
            device_name="mapfpgai";
    }else{
         if(1==reduce_num)
            device_name="reducefpga1";
        else if(2==reduce_num)
            device_name=="reducefpga2";
        else if(3==reduce_num )
            device_name="reducefpga3";
        else             
            device_name="reducefpgai";
    }
    fpga->cls=class_create(THIS_MODULE,device_name);
    if(!fpga->cls){
        printk(KERN_ERR "can't register class for fpga\n");
    }

    device_create(fpga->cls,NULL,fpga->cdevNum,NULL,device_name);

    printk(KERN_INFO "leave the set_chrdev function!\n");
    return 0;

    //zj:the problem is how to get the device

}

//probe the device and do following work
/**
 *enable the pci_device
 *
 *
 *
 *
 *
 */

static int probe(struct pci_dev *pdev, const struct pci_device_id *id) {
  
//Initalize driver info
    struct fpga_device *fpga = 0;

    printk(KERN_INFO "Entered driver probe function.\n");

    //printk(KERN_INFO " vendor = 0x%x, device = 0x%x \n", dev->vendor, dev->device);

    //Allocate and zero memory for fpga
    fpga = kmalloc(sizeof(struct fpga_device), GFP_KERNEL);
    if (!fpga) {
        printk(KERN_WARNING "Couldn't allocate memory for device info!\n");
        return -1;
    }else{
        printk(KERN_INFO "pdev is not null\n");
    }
    /* Clear the contents of the data structure */
    memset(fpga, 0, sizeof(struct fpga_device));
    if(pdev==NULL){
        printk(KERN_INFO "pdev is null, and there must be some error!\n");
    }
    //Copy in the pci device info1
    fpga->pdev = pdev;


    printk(KERN_INFO "init the sem");
    sema_init(&fpga->sem, 1);
    printk(KERN_INFO "sucess init the sem");

    //Save the device info itself into the pci driver

    //set the pci_dev->driver_data== fpga_device
    pci_set_drvdata(pdev, (void*) fpga);  //modify in this version 
	


    //Enable the PCI
    if (pci_enable_device(pdev)){
        printk(KERN_WARNING "pci_enable_device failed!\n");
        return -1;
    }
    //enable bus mastering
    pci_set_master(pdev);

    //enable
 
    //result=pci_set_consistent_dma_mask(pdev,DMA_BIT_MASK(32));  //set the mask for 64 address
   // result=dma_set_mask_and_coherent(pdev->dev,DMA_BIT_MASK(32) )
  if(!pci_set_dma_mask(pdev, DMA_BIT_MASK(64))) 
  {
     pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64));
     printk("Device setup for 64bit DMA\n");
  }
    /*
    result=pci_dac_set_dma_mask(pdev,DMA_BIT_MASK(64));
    if(result ==0){
        printk(KERN_INFO "DAC set scuess!\n");
    }*/
    //get get the mem of bar0
    fpga->len=pci_resource_len(pdev,0);
    fpga->flag=pci_resource_flags(pdev,0);
    printk(KERN_INFO "the size of io mem is %d\n", fpga->len);
    // use the upper information and reqsest the IO resourse

    printk(KERN_INFO "the io mem flag is %x\n",fpga->flag);
    if( (fpga->flag) & IORESOURCE_IO)
        printk(KERN_INFO "io \n");
    else if((fpga->flag) & IORESOURCE_MEM)
        printk(KERN_INFO "io mem\n");
    else if((fpga->flag) & IORESOURCE_PREFETCH)
        printk(KERN_INFO "the io mem is prefetch!\n");
    else if((fpga->flag) & IORESOURCE_READONLY)
        printk(KERN_INFO "ip mem is read only\n");
    else
    printk(KERN_INFO "other \n");

    //alloc the Io mem f PCIE BAR0
    if (NULL == request_mem_region(pci_resource_start(pdev,0), fpga->len, DRIVER_NAME)) {
       // printk(/*KERN_WARNING*/"%s: Init: Memory request failed.\n", DRIVER_NAME);
        return (-1);
    }
    printk(KERN_INFO "map the memory region\n");
    //remap the Io bar into the userspace

    fpga->baseAddr=ioremap(pci_resource_start(pdev,0),fpga->len);
    if(! fpga->baseAddr ){
        /*ioremap error*/
        printk(KERN_INFO "the ioremap failed\n");
        /*there shole be some code to deal with the error */
    }else{
        printk(KERN_INFO "the io baseAddr is %lx \n", fpga->baseAddr);
    }
   
    
    creat_buffer(fpga);
    //init the wait_queue_head_t
    init_waitqueue_head(&fpga->inq);
    init_waitqueue_head(&fpga->outq);
    
    
    //the code related to the Timer
    printk("add the timer sendTask\n");
    unsigned long j = jiffies;
    init_timer(&fpga->myTimer);
    fpga->myTimer.expires=j+TIME_DELAY*HZ;
    fpga->myTimer.data=(unsigned long)fpga;
    fpga->myTimer.function=&timerHandler;
    add_timer(&fpga->myTimer);
    
    int flag=ioread32(fpga->baseAddr + WORKFLAG);  //the type of job
    fpga->workflag=flag; 
    if(flag==1){
         map_num +=1;
         printk(KERN_INFO "This is a map device!\n");
    } 
    else{
        reduce_num +=1;
        printk(KERN_INFO "this is a reduce device! \n");
    }
    
        
    printk(KERN_INFO "begin register the char device\n");
    //Setup the char device
    /*##############################this may be modify,according to the README file ####################################*/
    setup_chrdev(fpga,flag);
	printk("scuess register the char device\n");
    
    //
    fpga->read_finished=1;
    fpga->write_finished=0;
    
    fpga->opencount=0;
   
    //for the test
    fpga->write_num=0;
    fpga->read_num=0;
    
    printk(KERN_INFO "exit the prob\n");
    return 0;  // if scuess return 0 else return fushu
}
/**remove the device and do following work
 *
 *
 */

static void remove(struct pci_dev *pdev) {

    struct fpga_device *fpga = 0;

    printk(KERN_INFO " Entered FPGA for bigdata driver remove function.\n");

    fpga = (struct fpga_device *) pci_get_drvdata(pdev);

    if (fpga == 0) {
    printk(KERN_WARNING "fpga for big data remove: fpga is 0");
        return;
    }
    //this should be done in the first ;
   del_timer(&fpga->myTimer);
   
    
    
    //release the mem_buffer
    destroy_buffer_all(fpga);
 
    if(fpga){
        /*call the fpga device release function*/
        //there may be need to modify########################################
        //release the major number
        printk(KERN_INFO "the major of device is %d", MAJOR(fpga->cdevNum));
        unregister_chrdev_region(fpga->cdevNum,1);
        //delete the device  from /dev
        device_destroy(fpga->cls,fpga->cdevNum);
        //delete the char device
        cdev_del(&(fpga->cdev));

        /*unmap the Io memory region*/
        if(fpga->baseAddr){
            printk(KERN_INFO "ifep: unmaping ioaddr \n");
            iounmap(fpga->baseAddr);
        }

        /*free the private data*/
        printk(KERN_ALERT "ifep:freeing card \n");
           //destroy the struct class
        class_destroy(fpga->cls);
        kfree(fpga);
     
    }



    //disable the device
    /* Clear the contents of the data structure */

    //unset the driver data
    pci_set_drvdata(pdev,NULL);

    /*Release the memory*/
    printk(KERN_ALERT "ifep: release mem region\n");
    release_mem_region(pci_resource_start(pdev, 0),
                       pci_resource_len(pdev, 0));
    pci_disable_device(pdev);

    printk(KERN_ALERT "fpga for bigdata: finished removing\n");
    printk(KERN_INFO "leave the remove function\n");

    //


}



//the module init function
static int fpga_init(void){
    //for test
    map_num=0;
    reduce_num=0;
    
    printk(KERN_INFO " Loading  FPGA_for_bigdata driver!\n");
    return pci_register_driver(&fpgaDriver);
}

//the module exit function
static void fpga_exit(void){
    
    printk(KERN_INFO "Exiting Fpga_for_bigdata driver!\n");
    pci_unregister_driver(&fpgaDriver);
}

module_init(fpga_init);
module_exit(fpga_exit);
