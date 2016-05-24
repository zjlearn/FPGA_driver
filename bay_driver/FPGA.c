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
int bayes_num;


//some declaration about the following function
static int probe(struct pci_dev *pdev, const struct pci_device_id *id);
static void remove(struct pci_dev *dev);


int fpga_open(struct inode *inode, struct file *filePtr);
int fpga_close(struct inode *inode, struct file *filePtr);
ssize_t fpga_read(struct file *filePtr, char __user *buf, size_t count, loff_t *pos);
ssize_t fpga_write(struct file *filePtr, const char __user *buf, size_t count, loff_t *pos);
int fpga_ioctl (struct inode *inode, struct file *filp,
                unsigned int cmd, unsigned long arg);
void sendTask(struct  fpga_device * fpga);

                
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
        case FPGA_IOCT:  /*this is for test time */
            printk(KERN_EMERG "the time information is following \n");
            if (fpga ->send_num == 0){
                printk(KERN_EMERG "has no send the task!\n");
                break;
            }
            int  t1=( fpga->send_time-fpga-> write_time)*1000/HZ/fpga->send_num;
            int t2=( fpga->result_time-fpga-> send_time)*1000/HZ/fpga->send_num;
            int  t3=( fpga->read_time- fpga-> result_time)*1000/HZ/fpga->send_num;
            printk(KERN_EMERG "has send %d times task\n", fpga->send_num);
            printk(KERN_EMERG "the avg write->sendtime is %d \n",t1);
            printk(KERN_EMERG "the avg send->result is %d \n",t2);
            printk(KERN_EMERG "the avg result->read is %d \n",t3);
           
            break;
        case FPGA_IOCSTATUS:  /*this is for get the buffer  status */
            if(fpga->send_num==0){
                printk(KERN_EMERG "not send task!\n");
                break;
            }
              printk(KERN_EMERG "the write has been blocked %d times \n",fpga->block_status);
              printk(KERN_EMERG "when send ,empty tx_buffer (no task ) occur %d times \n",fpga->empty_status);
              printk(KERN_EMERG "the average number of task in the tx_buffer  is %d \n", fpga->sum_task /fpga->send_num );
              
              
              break;
        case FPGA_IOCC:  /*this is to clear the buffer error status */
            refresh(fpga);
            printk(KERN_EMERG "the device is refreshed \n");
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

/*IRQ Function
*/

static irqreturn_t fpga_interrupt(int irq, void *dev_id,struct pt_regs *regs) //delete the parameter of  struct pt_regs *regs
{
    //disable irq, i am not sure this staement, i write this because the machine is reboot

    struct fpga_device *fpga;
    
    
   
   
   fpga=(struct fpga_device *)dev_id;   //modify in this version 2015/06/15   
   printk(KERN_INFO "enter the fpga_interrupt function!\n ");
   
   struct tx_buffer_struct * task=NULL;
   struct rx_buffer_struct * result_buffer=NULL;
   fpga->result_time=fpga->result_time +jiffies;
     
    fpga->write_finished=1;
    result_buffer=list_entry(fpga->rxEmpty,struct rx_buffer_struct,list);
    
    result_buffer->status=1;
    
    task=list_entry(fpga->txFull, struct tx_buffer_struct ,list);
    task->status=0;
    printk(KERN_INFO "write_finished ! ~~~~~~~~~~, the result buffer num is %d, the device is %s \n", result_buffer->num,fpga->devicename);
    //update the point rxAssign 
    fpga->rxEmpty=fpga->rxEmpty->next;
    fpga->txFull=fpga->txFull->next; 
    
    
    sendTask(fpga);
    
   //wake up the read function 
   wake_up_interruptible(&fpga->outq);
   wake_up_interruptible(&fpga->inq);
   
   
   
   return IRQ_HANDLED;
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
//   printk(KERN_INFO "fpga_open: Entering function.\n");
   
    fpga = container_of(inode->i_cdev,struct fpga_device,cdev);
    filePtr->private_data = fpga;
    return 0;
}

/**close function do the following work
 *
 *
 */
int fpga_close(struct inode *inode, struct file *filePtr){
    return 0;
}


/**
the function is to send the task into the device and relate result with the rx  buffer. 
*/
void sendTask(struct  fpga_device * fpga){
    
    //struct fpga_device * fpga=(struct fpga_device *)device;
    if(down_trylock(& (fpga->tx_sem)))  //we only try to lock the sem
    {
        printk(KERN_INFO "failed get the tx_sem\n");
        return ; 
    } 
    if(down_trylock(& (fpga->rx_sem)))
    {
        printk(KERN_INFO  "failed get the rx_sem  the device is %s \n", fpga->devicename);
        up(&(fpga->tx_sem));  //解除申请的tx_sem锁。
        return;
    }
    struct tx_buffer_struct * task=NULL;
    struct rx_buffer_struct * result_buffer=NULL;
    
   
    if( ! fpga->write_finished){
        printk(KERN_INFO "NO ready! the device is %s\n" , fpga->devicename);
        goto out;
    }
        
    //judge whether there some is data to send 
     task=find_tx_buffer(fpga,1);
    if(task == NULL)
    {
        fpga->empty_status=fpga->empty_status+1;
        printk(KERN_INFO "the task buffer is empty!");
        goto out;
    }
    
    fpga->send_num= fpga->send_num+1;
    fpga->send_time=fpga->send_time+jiffies;
    fpga->sum_task=fpga->sum_task+ fpga->write_num - fpga->send_num;
    
    
   

     u8 * flag=task->dma_buffer_tx;
     int message_flag = *(flag+31);
     printk(KERN_INFO "the flag is %d , the device is %s\n", message_flag, fpga->devicename);
    
    if(message_flag ==2 ){  //the task message
    
        //find whether there is some space to 
        result_buffer = find_rx_buffer(fpga ,0);
        
         if(result_buffer == NULL){
                printk(KERN_INFO "there is no result buffer !\n");
                goto out;
            }
            
        //update the ready 
        fpga->write_finished=0;
          
        printk(KERN_INFO "the process %d ,the task is to send in the %d tx_buffer!, result is send into %d rx_buffer ,the device is %s , there is %d task in the tx_buffer\n",task->pid, task->num, result_buffer->num, fpga->devicename,fpga->write_num - fpga->send_num);
      
         //get the docN and compute the size of result   
         u32 *docN_point=flag+27;
         u32 docN=*docN_point;
         result_buffer->len=docN*32;     //计算结果为文档数的32倍字节这么大。
         
         //send the data and update the tx_buffer and rx_buffer
        iowrite32(task->dma_bus_tx , fpga->baseAddr + READADDRLOW_REG);
        u64 high_tx_address=(task->dma_bus_tx )>>32;
        iowrite32(high_tx_address , fpga->baseAddr + READADDRHIGH_REG);
        iowrite32(task->len, fpga->baseAddr + READDATALEN_REG);
        wmb(); //to ensure the operation of write not to change the order
       
        task->status=-1;  //that mean  the data of this  buffer is in  transfers 
        
        //write the result address into the reg
        iowrite32(result_buffer->dma_bus_rx, fpga->baseAddr + WRITEADDRLOW_REG);
        u64 high_rx_address=result_buffer->dma_bus_rx>>32;
        iowrite32(high_rx_address, fpga->baseAddr + WRITEADDRHIGH_REG);
      
        
        result_buffer->status= -1;  //meaning this buffer has been assigned;
        result_buffer->pid=task->pid;
    }else{ //the prob message
        printk(KERN_INFO "the process %d ,the task is to send in the %d tx_buffer!, the device is %s , there is %d task in the tx_buffer\n",task->pid, task->num, fpga->devicename,fpga->write_num - fpga->send_num);

        //send the data and update the tx_buffer and rx_buffer
        iowrite32(task->dma_bus_tx , fpga->baseAddr + READADDRLOW_REG);
        u64 high_tx_address=(task->dma_bus_tx )>>32;
        iowrite32(high_tx_address , fpga->baseAddr + READADDRHIGH_REG);
        iowrite32(task->len, fpga->baseAddr + READDATALEN_REG);
        wmb(); //to ensure the operation of write not to change the order
        
        
        //state has write finished the task
        fpga->write_finished=1;
        task=list_entry(fpga->txFull, struct tx_buffer_struct ,list);
        task->status=0;
        fpga->txFull=fpga->txFull->next;
    }
    
   
    
    //启动DMA
    iowrite32(START_DMA,fpga->baseAddr+DMA_ENABLE_REG);

    up(&(fpga->tx_sem));
    up(&(fpga->rx_sem));
  
    return ;
    
 out:  //send  failed。 should release the semaphore we alloc . 
   // printk(KERN_INFO "error in the sendTask function !\n");
    up(&(fpga->tx_sem));
    up(&(fpga->rx_sem));
    return;
    
}

//Pass-through to main dispatcher
/**the read  function do the following work
 * 
 * 
 */
ssize_t fpga_read(struct file *filePtr, char __user *buf, size_t count, loff_t *pos){

   
    struct fpga_device *fpga=filePtr->private_data;

    rxlist  rx_buffer;
    
    printk(KERN_INFO "the process %d is in the fpga_read function --------, the device is %s \n", current->tgid, fpga->devicename);
   // printk(KERN_INFO "the current process is %s .",current->comm);
    //I am not sure whether it is right to get the sem like that 
    if(down_interruptible(&fpga->rx_sem))
        return -ERESTARTSYS;
    while(find_rx_buffer(fpga,1)== NULL ){  //没有本进程的数据可以读
        up(&fpga->rx_sem);
        printk(KERN_INFO " \" %s\" reading : going to sleep\n", current->comm);
        if(wait_event_interruptible(fpga->outq,find_rx_buffer(fpga,1)!=NULL))
            return -ERESTARTSYS; //信号，通知fs层做相应的处理
        //否则循环，但首先获得锁
        if(down_interruptible(&fpga->rx_sem))
            return -ERESTARTSYS;
    }
    /*the data is  ready , and return */
    rx_buffer=find_rx_buffer(fpga,1);
    if(rx_buffer ==NULL ){
        printk(KERN_INFO "there is some wrong in the fpga_read function!\n");
         up(&fpga->rx_sem);
        return -EFAULT;
    }
    fpga->read_time=fpga->read_time+jiffies;
    unsigned long result_size=count;
    if(fpga->workflag==3){ //the device is bayes_fpga
        result_size=rx_buffer->len;
    }
    //the data is ready,
    //read from the data from the buffer 
    if(copy_to_user(buf,rx_buffer->dma_buffer_rx,result_size)){ //copy the data to the user directly
        up(&fpga->rx_sem);
        printk(KERN_INFO "the copy_to_user is some thing wrong！###################\n");
        return -EFAULT;
    }
    printk(KERN_INFO "the process %d get the result from the %d rx_buffer! the device is %s  \n",current->tgid,rx_buffer->num,fpga->devicename);
    fpga->read_num =fpga->read_num+1;
   
    //update the status info       
    rx_buffer->status=0;     
    //update the point
    fpga->rxFull=fpga->rxFull->next;
    //some unlock, this shouldshould be modified in the later
    up(&fpga->rx_sem);
    
    sendTask(fpga);
    return result_size;
}



/**
*this is get the write space 
*this function deal with the block and the sem 
* if scuess(has some space to use ) return zero 
  else return not zero
*/
static int  getwritespace(struct fpga_device * fpga){
   // printk(KERN_INFO "enter the getwritespace function \n");
    while(find_tx_buffer(fpga,0)==NULL){ //full
    
        DEFINE_WAIT(wait);
        up(&fpga->tx_sem);
        
      //  printk(KERN_INFO "\" %s \" the writing is going to sleep \n",current->comm);
        prepare_to_wait_exclusive(&fpga->inq,&wait,TASK_INTERRUPTIBLE);
        //prepare_to_wait(&fpga->inq,&wait,TASK_INTERRUPTIBLE);
        if(find_tx_buffer(fpga,0)==NULL)
            schedule();
        
        fpga->block_status =fpga->block_status+1;
        
        printk(KERN_INFO  "write function end the finished $$$$$$$$$$$$$$$$$$$$$$$$$$$$$!\n");
        finish_wait(&fpga->inq,&wait);
        if(signal_pending(current))
        {
            return -ERESTARTSYS;
        }  
        if(down_interruptible(&fpga->tx_sem)){
            return -ERESTARTSYS;
        }   
    }
   //printk(KERN_INFO "leave the getwritespace function \n");
    return 0;
}


/**the write function do the following work
 * get a buffer to store the data 
 *
 */
 
ssize_t fpga_write(struct file *filePtr, const char __user *buf, size_t count, loff_t *pos){
   
    int result;
    
    struct fpga_device * fpga =filePtr->private_data; 
    
    txlist  tx_buffer;
    
    fpga->write_time =fpga->write_time +jiffies;
    
    //get the lock
    if( down_interruptible(&fpga->tx_sem))
    {
        printk(KERN_EMERG "can't get the sem\n ");
        return -ERESTARTSYS;
    }
    result =getwritespace(fpga);
    if(result){  
       // printk(KERN_INFO "the error \n");  //
        printk(KERN_INFO " can't get the buffer and write return , the device is %s\n", fpga->devicename);
        goto out;
    }
    printk(KERN_INFO "the process %d is in the fpga_write function+++++++++, the device is %s ", current->tgid, fpga->devicename);
    
   // printk(KERN_INFO "the current processs is %s",current->comm);
    tx_buffer=find_tx_buffer(fpga,0);
    if(tx_buffer==NULL){
        printk(KERN_EMERG "error in the find_tx_buffer\n");
        up(&fpga->tx_sem);
        return -EFAULT;
    }
    if( count > TX_BUFFER_SIZED ){
        printk(KERN_EMERG "error  the data is large than capacity!\n");
        up(&fpga->tx_sem);
        return -EFAULT;
    }
    
    if(copy_from_user(tx_buffer->dma_buffer_tx,buf,count))//copy the data to the user directly
    {
        printk(KERN_EMERG "error in the copy_from_user  in the fpga_write function \n  ");
        up(&fpga->tx_sem);
		return -EFAULT;
    }
    
    printk(KERN_INFO "the process %d has writen the data into the %d num, the device is %s \n ", current->tgid,tx_buffer->num, fpga->devicename);
    tx_buffer->status=1;  //update the status of buffer
    tx_buffer->len=count;
   
   fpga->write_num = fpga->write_num+1;
   

    tx_buffer->pid=current->tgid;
    //update the status
    fpga->txEmpty= fpga->txEmpty->next;
    
    up(&fpga->tx_sem);

    sendTask(fpga);
    
    
    return count;
    out:
     sendTask(fpga);
     return result; 
}

//setup the char device
static int setup_chrdev(struct fpga_device *fpga,int flag){
    /*
    Setup the /dev/deviceName to allow user programs to read/write to the driver.
    */
   //  printk(KERN_INFO "Enter the setup_chrdev function\n");
          
    int devMinor =0 ;
   
    int devMajor =101+map_num+reduce_num+bayes_num;
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
        switch(map_num){
            case 1: device_name="mapfpga1"; break;
            case 2: device_name="mapfpga2"; break;
            default: device_name="mapfpgai";
        }
    }else if(flag==2){
         switch(reduce_num){
            case 1: device_name="reducefpga1"; break;
            case 2: device_name="reducefpga2"; break;
            default: device_name="reducefpgai";
        }        
    }else{
        switch(bayes_num){
            case 1: device_name="bayesfpga1"; break;
            case 2: device_name="bayesfpga2"; break;
            default: device_name="bayesfpgai";
        }   
    }
    
    fpga->devicename=device_name;
    fpga->cls=class_create(THIS_MODULE,device_name);
    if(!fpga->cls){
        printk(KERN_ERR "can't register class for fpga\n");
    }

    device_create(fpga->cls,NULL,fpga->cdevNum,NULL,device_name);

   // printk(KERN_INFO "leave the set_chrdev function!\n");
    return 0;

    //zj:the problem is how to get the device

}

int  sendTask_thread(void * data ){
    while(1){
        if( kthread_should_stop())  return -1;  
  
    struct fpga_device * fpga = (struct fpga_device * )data;
  //struct fpga_device * fpga=(struct fpga_device *)device;
    if(down_trylock(& (fpga->tx_sem)))  //we only try to lock the sem
    {
        printk(KERN_INFO "failed get the tx_sem\n");
         goto outtwo;
    } 
    if(down_trylock(& (fpga->rx_sem)))
    {
        printk(KERN_INFO  "failed get the rx_sem  the device is %s \n", fpga->devicename);
        up(&(fpga->tx_sem));  //解除申请的tx_sem锁。
         goto outtwo;
    }
    struct tx_buffer_struct * task=NULL;
    struct rx_buffer_struct * result_buffer=NULL;
    
   
    if( ! fpga->write_finished){
        printk(KERN_INFO "NO ready! the device is %s\n" , fpga->devicename);
        goto outone;
    }
        
    //judge whether there some is data to send 
     task=find_tx_buffer(fpga,1);
    if(task == NULL)
    {
        printk(KERN_INFO "the task buffer is empty the device is %s\n" , fpga->devicename);
        fpga->empty_status=fpga->empty_status+1;
        goto outone;
    }
    
    fpga->send_num= fpga->send_num+1;
    fpga->send_time=fpga->send_time+jiffies;
    fpga->sum_task=fpga->sum_task+ fpga->write_num - fpga->send_num;
    
    
   

     u8 * flag=task->dma_buffer_tx;
     int message_flag = *(flag+31);
     printk(KERN_INFO "the flag is %d , the device is %s\n", message_flag, fpga->devicename);
    
    if(message_flag ==2 ){  //the task message
    
        //find whether there is some space to 
        result_buffer = find_rx_buffer(fpga ,0);
        
         if(result_buffer == NULL){
                printk(KERN_INFO "there is no result buffer ! the device is %s \n", fpga->devicename);
                goto outone;
            }
            
        //update the ready 
        fpga->write_finished=0;
          
        printk(KERN_INFO "the process %d ,the task is to send in the %d tx_buffer!, result is send into %d rx_buffer ,the device is %s , there is %d task in the tx_buffer\n",task->pid, task->num, result_buffer->num, fpga->devicename,fpga->write_num - fpga->send_num);
      
         //get the docN and compute the size of result   
         u32 *docN_point=flag+27;
         u32 docN=*docN_point;
         result_buffer->len=docN*32;     //计算结果为文档数的32倍字节这么大。
         
         //send the data and update the tx_buffer and rx_buffer
        iowrite32(task->dma_bus_tx , fpga->baseAddr + READADDRLOW_REG);
        u64 high_tx_address=(task->dma_bus_tx )>>32;
        iowrite32(high_tx_address , fpga->baseAddr + READADDRHIGH_REG);
        iowrite32(task->len, fpga->baseAddr + READDATALEN_REG);
        wmb(); //to ensure the operation of write not to change the order
       
        task->status=-1;  //that mean  the data of this  buffer is in  transfers 
        
        //write the result address into the reg
        iowrite32(result_buffer->dma_bus_rx, fpga->baseAddr + WRITEADDRLOW_REG);
        u64 high_rx_address=result_buffer->dma_bus_rx>>32;
        iowrite32(high_rx_address, fpga->baseAddr + WRITEADDRHIGH_REG);
      
        
        result_buffer->status= -1;  //meaning this buffer has been assigned;
        result_buffer->pid=task->pid;
    }else{ //the prob message
        printk(KERN_INFO "the process %d ,the task is to send in the %d tx_buffer!, the device is %s , there is %d task in the tx_buffer\n",task->pid, task->num, fpga->devicename,fpga->write_num - fpga->send_num);

        //send the data and update the tx_buffer and rx_buffer
        iowrite32(task->dma_bus_tx , fpga->baseAddr + READADDRLOW_REG);
        u64 high_tx_address=(task->dma_bus_tx )>>32;
        iowrite32(high_tx_address , fpga->baseAddr + READADDRHIGH_REG);
        iowrite32(task->len, fpga->baseAddr + READDATALEN_REG);
        wmb(); //to ensure the operation of write not to change the order
        
        
        //state has write finished the task
        fpga->write_finished=1;
        task=list_entry(fpga->txFull, struct tx_buffer_struct ,list);
        task->status=0;
        fpga->txFull=fpga->txFull->next;
        
       
    }
    
   
    
    //启动DMA
    iowrite32(START_DMA,fpga->baseAddr+DMA_ENABLE_REG);
     outone:  
            up(&(fpga->tx_sem));
            up(&(fpga->rx_sem));
            //唤醒一些读写进程
            wake_up_interruptible(&fpga->inq);
            wake_up_interruptible(&fpga->outq);
            
     outtwo: 
            set_current_state(TASK_UNINTERRUPTIBLE);  
            schedule_timeout(0.001*HZ);
    }
    return 0;
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


    //Allocate and zero memory for fpga
    fpga = kmalloc(sizeof(struct fpga_device), GFP_KERNEL);
    if (!fpga) {
        printk(KERN_WARNING "Couldn't allocate memory for device info!\n");
        return -1;
    }else{
        printk(KERN_INFO "pdev is not null\n");
    }
    /* Clear the contents of the data structure */
    //memset(fpga, 0, sizeof(struct fpga_device));
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
  if(!pci_set_dma_mask(pdev, DMA_BIT_MASK(64))) 
  {
     pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64));
     printk("Device setup for 64bit DMA\n");
  }
    
    //get get the mem of bar0
    fpga->len=pci_resource_len(pdev,0);
    fpga->flag=pci_resource_flags(pdev,0);
    printk(KERN_INFO "the size of io mem is %d\n", fpga->len);
    // use the upper information and reqsest the IO resourse

    

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
    
    //
   
    fpga->write_finished=1;
    

    
    int flag=ioread32(fpga->baseAddr + WORKFLAG);  //the type of job
    fpga->workflag=flag; 
    if(flag==1){
         map_num +=1;
         printk(KERN_INFO "This is a map device!\n");
    } 
    else if(flag==2){
        reduce_num +=1;
        printk(KERN_INFO "this is a reduce device! \n");
    }else{
        bayes_num+=1;
    }
    
        
    printk(KERN_INFO "begin register the char device\n");
    //Setup the char device
    /*##############################this may be modify,according to the README file ####################################*/
    setup_chrdev(fpga,flag);
	printk("scuess register the char device\n");
    
   
   
    //for the test
    fpga->write_num=0;
    fpga->read_num=0;
    fpga->send_num=0;
        
    fpga->write_time=0;
    fpga->send_time=0;
    fpga->result_time=0;
    fpga->read_time=0;

    fpga->empty_status=0;
    fpga->block_status=0;
    fpga->sum_task=0;
    
    
    //alloc the irq
    fpga->ms_entey=kmalloc(sizeof(struct msix_entey *)*MSIX_NUM, GFP_KERNEL);
    fpga->ms_entey[0].entry=0;
    fpga->ms_entey[1].entry=1;
    fpga->ms_entey[2].entry=2;
    fpga->ms_entey[3].entry=3;
    fpga->ms_entey[4].entry=4;
   int err=-1;
   int nvec=5;
   int rc;
   while(nvec>=1){
       rc=pci_enable_msix(fpga->pdev,fpga->ms_entey, nvec);
       if(rc>0)
           nvec=rc;
       else{
           if (rc==0)
               break;
           else
            printk(KERN_EMERG "pci_enable_msix alloc disable");
       }
   }
       
   printk(KERN_EMERG "alloc the msix sucess\n");
    
   err = request_irq(fpga->ms_entey[0].vector, fpga_interrupt, IRQF_TRIGGER_RISING ,DEVICE_NAME, fpga); 
   if(err==0){
       printk(KERN_INFO "request zero irq scuess!\n");
   }else{
       printk(KERN_EMERG"request  irq failed!\n");
   }
   
     /* kernel  thread */
    fpga->sendTask=kthread_create(sendTask_thread,fpga,"mykthread");  
    wake_up_process(fpga->sendTask);

   
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
    
   //
   free_irq(fpga->ms_entey[0].vector,fpga);
   pci_disable_msix(fpga->pdev);
   
    
    
    //release the mem_buffer
    destroy_buffer_all(fpga);
    
   //关闭内核线程
   kthread_stop(fpga->sendTask);
 
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
