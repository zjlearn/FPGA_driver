//
// Created by zj on 2015/4/23.
//this is the pcie_Fpga driver
//

#ifndef FPGA_DRIVER_FPGA_H
#define FPGA_DRIVER_FPGA_H


#define TIME_DELAY  0.01  //this means that timer is delayed 5 seconds

#include<linux/kthread.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include<linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/wait.h>  //for wait_queue_head_t
#include<linux/list.h>  //for the data struct list_head

//for the timer
#include<linux/timer.h>
#include <linux/jiffies.h>

#include "buffer.h" 

//this  is  the config var
#define VENDER_ID_ONE  0x19e5
#define DEVICE_ID_ONE  0x0164
#define VENDER_ID_TWO 0x19e5
#define  DEVICE_ID_TWO 0x0165


#define DRIVER_NAME "fpaga_driver"
#define DEVICE_NAME "FPGA_for_bigdata"

#define BOARD_NAME  "ifep_board"

#define  MSIX_NUM 5

#define  MAX_DATA_SIZE   4000  /*the max data to transmission; and we get a 4MB space to storage data*/

#define WORKFLAG        0x118    /*the type of work , one mean map device and two mean reduce*/

//the following include some reg
#define VERSION_REG     0x100   /*the version of logic*/
#define DMA_ENABLE_REG  0x104         /*DMA enable reg*/
#define INTERRUPTSTATUS_REG    0x114    /*represent the error status, and this var is reserved*/
#define READY_REG          0x110  /*represent the FPGA can recieve data from cpu*/

#define WRITEADDRHIGH_REG   0x504  /*the high address of write*/
#define WRITEADDRLOW_REG   0x500   /*the low addr*/
#define WRITEFIN_REG        0x508   /**represent finish write to the RAM*/

#define READADDRHIGH_REG   0x604 /*the high addr of read*/
#define READADDRLOW_REG  0x600 /*the low addr of read*/
#define READDATALEN_REG  0x608   /*the len of data to read*/
#define RESET_REG   0x404        /*reset the bar reg*/

/*the value Corresponding to the INTERRUPTSTATUS_REG*/
#define INT_DMA_W_COMPLETE 0x1   
#define INT_DMA_R_COMPLETE 0x2

#define DMA_READY   0x1    /*Corresponding to the READY_REG*/
#define START_DMA   0x1

//*********************************the var and function about the buffer
//Ring of receive buffers
typedef struct rx_buffer_struct{
    void * dma_buffer_rx;  //the kernel virtual address buffer
    dma_addr_t dma_bus_rx; //bus address of the receive buffer
    struct list_head list;
    int len;            //the len of the data
    int status;   //busy or free
    int pid;      // the process pid which this buffer belong to
    int num;      //this is for test
}* rxlist;



//Ring of transmit buffers
typedef struct tx_buffer_struct{
    void * dma_buffer_tx;
    dma_addr_t dma_bus_tx;
    struct list_head list;
    int len;
    int status;
    int pid;
    int num; //this is for the test;
}* txlist;


//these is some annomention about the function

struct fpga_device{
    struct pci_dev * pdev;

    struct device *device;
  
    //mutex for this device
    struct semaphore sem;

    //the resourse var. i.e the bar space
    void * baseAddr;   /*the cirtual address after ioremap*/
    u32 len;
    u32 flag;
 
    //char dev 
    struct cdev cdev;   /*represent the char device*/
    dev_t cdevNum; /*represent the major and minor number */
    struct class *cls;
    
    
    //some wait queue for other process
    wait_queue_head_t inq;
    wait_queue_head_t outq; 
    
    
    //the rx and the tx buffer list
    rxlist rx_buffer;
    txlist tx_buffer;

    //mutex for this device rx/tx buffer
    struct semaphore rx_sem;
    struct semaphore tx_sem;
        
    //the point relate to the rx_buffer 
    struct list_head * rxFull; //this means there is some data in the buffer
    struct list_head * rxEmpty;  //this means this buffer is assign but there is no data
    
    
    //the point realate to tx_buffer
    struct list_head * txEmpty;
    struct list_head *txFull;
 
    //means the fpag read or write data finished
    int write_finished;
   
    //represent the type of device_work. 1 means map ; 2 means reduce
    int workflag;
    
     /*the kernel thread*/
     struct task_struct * sendTask;
    
    //the var for test
    int write_num;
    int read_num;
    int send_num;
    
    char * devicename;
    //
    struct msix_entry * ms_entey;
    
    unsigned long  write_time;
    unsigned long  send_time;
    unsigned long  result_time;
    unsigned long  read_time;
    
    int empty_status;
    int block_status;
    int sum_task;   //average the task in the tx_buffer;
};

void test(struct fpga_device * fpga);

int creat_buffer(struct fpga_device * fpga);
int destroy_buffer_all(struct fpga_device * fpga);

struct rx_buffer_struct * find_rx_buffer(struct fpga_device * fpga, int flag);
struct tx_buffer_struct * find_tx_buffer(struct fpga_device * fpga, int flag);

int fpga_kmem_free_rbuffer(struct fpga_device *, rxlist);
int fpga_kmem_free_tbuffer(struct fpga_device * fpga, txlist tbuffer);
void refresh(struct fpga_device * fpga);

#endif //FPGA_DRIVER_FPGA_H

