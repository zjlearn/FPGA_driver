/*this codeis a temp code for the many buffer
*/

/*
date:2015/06/09
auth:zj
*/
/*
the buffer may use streaming buffer 
but it may be  suitable and Efficientto use consistent buffer-----zj  2015/06/09
*/

#include "FPGA.h"

#include "buffer.h"



//this is a test function
void test(struct fpga_device * fpga){
    return 0; //fro test
    
    
    struct list_head * ptr,*next;
    rxlist rbuffer;
    txlist tbuffer;
    printk(KERN_INFO "enter the test function \n");
    //print the num zero rx buffer 
    rbuffer=fpga->rx_buffer;
    printk(KERN_INFO "rx buffer. the num is %d , the pid is %d , \
                the status is %d , the len is %d , the bus addr is %lx , the virt addr is %lx \n", 
                rbuffer->num ,rbuffer->pid,rbuffer->status,rbuffer->len,rbuffer->dma_bus_rx,rbuffer->dma_buffer_rx);
    /* iterate safely over the */
    list_for_each_safe(ptr,next,&(fpga->rx_buffer->list)){
        rbuffer=list_entry(ptr,struct rx_buffer_struct,list);
        printk(KERN_INFO "rx buffer. the num is %d , the pid is %d , \
                the status is %d , the len is %d , the bus addr is %lx , the virt addr is %lx \n", 
                rbuffer->num ,rbuffer->pid,rbuffer->status,rbuffer->len,rbuffer->dma_bus_rx,rbuffer->dma_buffer_rx);
    }
    //print the number zero  tx buffer 
    tbuffer=fpga->tx_buffer;
    printk(KERN_INFO "tx buffet. the num is %d , the pid is %d , \
                the status is %d , the len is %d , the bus addr is %lx , the virt addr is %lx \n", 
                tbuffer->num ,tbuffer->pid,tbuffer->status,tbuffer->len,tbuffer->dma_bus_tx,tbuffer->dma_buffer_tx);
    //print the other buffer
    list_for_each_safe(ptr,next,&(fpga->tx_buffer->list)){
        tbuffer=list_entry(ptr,struct tx_buffer_struct,list);
          printk(KERN_INFO "tx buffet. the num is %d , the pid is %d , \
                the status is %d , the len is %d , the bus addr is %lx , the virt addr is %lx \n", 
                tbuffer->num ,tbuffer->pid,tbuffer->status,tbuffer->len,tbuffer->dma_bus_tx,tbuffer->dma_buffer_tx);
    }
    /*test the point */
 
    printk(KERN_INFO "leave the test function! \n");
    
    
}


/*this function is to refresh the driver (clear the state!)
*/
void refresh(struct fpga_device * fpga){
    //
    fpga->write_num=0;
    fpga->read_num=0;
    fpga->send_num=0;
    
    fpga->write_finished=1;
    
     fpga->write_time=0;
    fpga->result_time=0;
    fpga->send_time=0;
    fpga->read_time=0;
            
    fpga->block_status=0;
    fpga->empty_status=0;
    fpga->sum_task=0;
    
    //clear the buffer state
   struct list_head * ptr,*next;
    rxlist rbuffer;
    txlist tbuffer;
    
    rbuffer=fpga->rx_buffer;
    rbuffer->pid=0;
    rbuffer->status=0;
    rbuffer->len=0;
    list_for_each_safe(ptr,next,&(fpga->rx_buffer->list)){
        rbuffer=list_entry(ptr,struct rx_buffer_struct,list);
        rbuffer->pid=0;
        rbuffer->status=0;
        rbuffer->len=0;
    }
    
    tbuffer=fpga->tx_buffer;
    tbuffer->pid=0;
    tbuffer->len=0;
    tbuffer->status=0;
    list_for_each_safe(ptr,next,&(fpga->tx_buffer->list)){
        tbuffer=list_entry(ptr,struct tx_buffer_struct,list);
        tbuffer->pid=0;
        tbuffer->len=0;
        tbuffer->status=0;
    }
    //reset the point 
    fpga->rxFull=&(fpga->rx_buffer->list);
    fpga->rxEmpty= &( fpga->rx_buffer->list);
    
    fpga->txEmpty= &(fpga->tx_buffer->list);
    fpga->txFull=&(fpga->tx_buffer->list);
              
}

/*
*creat the receive buffers and the transmit buffers
*/
int creat_buffer(struct fpga_device * fpga){
    rxlist r_buffer;
    txlist t_buffer;
    int i;
    //init the fpga->rx_buffer and tx_buffer

    r_buffer =kmalloc(sizeof(struct rx_buffer_struct),GFP_KERNEL);
    r_buffer->dma_buffer_rx = pci_alloc_consistent(fpga->pdev, RX_BUFFER_SIZE, &(r_buffer->dma_bus_rx));
    
    if(r_buffer->dma_buffer_rx==NULL){ //the alloc is failed
        goto out;
        printk(KERN_INFO "no mem, goto the out \n");
    }
  
    
    r_buffer->len=0;
    r_buffer->status=0;
    r_buffer->pid=0;
    r_buffer->num=0;
    
       
    t_buffer=kmalloc(sizeof(struct tx_buffer_struct),GFP_KERNEL);
    //do some the initial
    t_buffer->dma_buffer_tx=pci_alloc_consistent(fpga->pdev, TX_BUFFER_SIZED,&(t_buffer->dma_bus_tx));
    if( ! t_buffer->dma_buffer_tx){
        printk(KERN_INFO "no mem, alloc the mem using the dma_alloc_coherent  failed \n");
    }
    t_buffer->len=0;
    t_buffer->status=0;
    t_buffer->pid=0;
    t_buffer->num=0;
    
    fpga->rx_buffer=r_buffer;
    fpga->tx_buffer=t_buffer;
    
    //create the rx_pool list
    printk(KERN_INFO "creat the  cycle buffer!\n");
    printk(KERN_INFO "the size of tx_buffer is %d \n", TX_BUFFER_SIZED);
    //INNIT the list head , in the program , the head is a elem
    INIT_LIST_HEAD( &(fpga->rx_buffer->list) );
    INIT_LIST_HEAD(&(fpga->tx_buffer->list));
    
    for (i=1 ; i<=NUM_RX_BUFFS; i++){
        r_buffer=kmalloc(sizeof(struct rx_buffer_struct),GFP_KERNEL);
        //do some the initial
        r_buffer->dma_buffer_rx = pci_alloc_consistent(fpga->pdev, RX_BUFFER_SIZE, &(r_buffer->dma_bus_rx));
        if(r_buffer->dma_buffer_rx==NULL){ //the alloc is failed
            printk(KERN_INFO "no mem, alloc the mem failed  in %d rx_buffer \n", i); 
            goto out;
        
        }else{
            printk(KERN_INFO "create the %d rx_buffer scuess! \n", i);
        }
   
        
        r_buffer->len=0;
        r_buffer->status=0;
        r_buffer->pid=0;
        r_buffer->num=i;
    
    //add to the list
    //printk(KERN_INFO "add the buffer into the list ");
    list_add_tail(&(r_buffer->list), &(fpga->rx_buffer->list));
    }
    
    //create the tx_pool list
   
    for (i=1; i<=NUM_TX_BUFFS; i++){
    t_buffer=kmalloc(sizeof(struct tx_buffer_struct),GFP_KERNEL);
    //do some the initial
    t_buffer->dma_buffer_tx=pci_alloc_consistent(fpga->pdev, TX_BUFFER_SIZED,&(t_buffer->dma_bus_tx));
    if( ! t_buffer->dma_buffer_tx){
        printk(KERN_INFO  "no mem, alloc the mem failed  in %d tx_buffer \n", i);
    }
    else{
        printk(KERN_INFO "creat the %d  tx_buffer scuess !\n" , i);
    }
    t_buffer->len=0;
    t_buffer->status=0;
    t_buffer->pid=0;
    t_buffer->num=i;
    //add to the list
    list_add_tail( &(t_buffer->list), &(fpga->tx_buffer->list));
    }

    //do the initial for the var related to  the buffer
    sema_init(&fpga->rx_sem,1);
    sema_init(&fpga->tx_sem,1);
    
    fpga->rxFull=&(fpga->rx_buffer->list);
    fpga->rxEmpty= &( fpga->rx_buffer->list);
    
    fpga->txEmpty= &(fpga->tx_buffer->list);
    fpga->txFull=&(fpga->tx_buffer->list);
    
     //this is for a test, after build cycle buffer scuess and we loop it and printk
     printk(KERN_INFO "test for the alloc process !\n");
     test(fpga);
     return 0;
    //do some error handle(上面的内存申请过程，并不一定总是能够成功)  using goto syntax
     //we should modify this in the future
    out: printk(KERN_INFO "deal with the error! in  the out\n");
    printk(KERN_INFO "leave the creat_buffer function\n");
   
}

/*destroy the rx and tx buffer*/
int  destroy_buffer_all(struct fpga_device * fpga){

    struct list_head * ptr,*next;
    rxlist rbuffer;
    txlist tbuffer;
    printk(KERN_INFO "enter the destroy_buffer_all function ! \n");
    
    /* iterate safely over the entries and delete them */
    list_for_each_safe(ptr,next,&(fpga->rx_buffer->list)){
        rbuffer=list_entry(ptr,struct rx_buffer_struct,list);
        fpga_kmem_free_rbuffer(fpga, rbuffer);      /*deal with the lock inside */
    }
    
     list_for_each_safe(ptr,next,&(fpga->tx_buffer->list)){
        tbuffer=list_entry(ptr,struct tx_buffer_struct,list);
        fpga_kmem_free_tbuffer(fpga, tbuffer);      /*deal with the lock inside */
    }
    //delete the first  buffer
    rbuffer=fpga->rx_buffer;
    printk(KERN_INFO "free the %d r_buffer \n", rbuffer->num);
    pci_free_consistent(fpga->pdev,RX_BUFFER_SIZE,rbuffer->dma_buffer_rx, rbuffer->dma_bus_rx);
    kfree(rbuffer);
    
    
    tbuffer=fpga->tx_buffer;
    printk(KERN_INFO "free the %d t_buffer \n", tbuffer->num);
    pci_free_consistent(fpga->pdev,TX_BUFFER_SIZED,tbuffer->dma_buffer_tx, tbuffer->dma_bus_tx);
    kfree(tbuffer);
    
    
    
    
    //do other var  reset in the device
    fpga->rxEmpty=NULL;
    fpga->rxFull=NULL;
    
    fpga->txEmpty=NULL;
    fpga->txFull=NULL;
    
    printk(KERN_INFO "leave the destroy_buffer_all function! \n");
    return 0;
}

//release the r_buffer
int fpga_kmem_free_rbuffer(struct fpga_device * fpga, rxlist rbuffer){

    //get the list lock
    down_interruptible(&fpga->rx_sem);
    
    //free the DMA buffer
    pci_free_consistent(fpga->pdev,RX_BUFFER_SIZE,rbuffer->dma_buffer_rx, rbuffer->dma_bus_rx);
    printk(KERN_INFO "free the %d r_buffer \n", rbuffer->num);
    list_del( & (rbuffer->list) );  //delete the list
    //release the rxlist memccpy
    kfree(rbuffer);

    up(&fpga->rx_sem);
}

//release the t_buffer
int fpga_kmem_free_tbuffer(struct fpga_device * fpga, txlist tbuffer){

    //get the list lock
   
    down_interruptible(&fpga->tx_sem);
    //free the DMA buffer
    pci_free_consistent(fpga->pdev,TX_BUFFER_SIZED,tbuffer->dma_buffer_tx, tbuffer->dma_bus_tx);
    printk(KERN_INFO "free the %d t_buffer \n", tbuffer->num);
    list_del( & (tbuffer->list) );  //delete the list
    //release the rxlist memccpy
    kfree(tbuffer);
    up(&fpga->tx_sem);
    
}

/*
*    flag==0 : find the rx_buffer is empty
*    flag==1 : find the rx_buffer is full
*   id may be the process id or thread id . 
*   id  affect when the flag==1. it mean find the result data related to the id 
*   if find failed, then return NULL 
*     
*/
struct rx_buffer_struct * find_rx_buffer(struct fpga_device * fpga, int flag){
        //printk(KERN_INFO "--------------find rx_buffer!\n ");
        struct rx_buffer_struct * rx_buffer =NULL;  
        if(flag==0){  //get the empty rx_buffer
           // printk(KERN_INFO "want to get the empty buffer!\n");
            rx_buffer=list_entry(fpga->rxEmpty,struct rx_buffer_struct, list);  //get the buffer point
           // printk(KERN_INFO "the erx_buffer num is %d \n",rx_buffer->num);
            if(rx_buffer->status != 0)
                rx_buffer=NULL;
            
        }else{ //get the full rx_buffer
           // printk(KERN_INFO "want to get the full rx_buffer!\n");
            rx_buffer=list_entry(fpga->rxFull,struct rx_buffer_struct, list);  //get the buffer point
                if(rx_buffer->status != 1)
                {
                   // printk("the rxFull buffer is %d\n", rx_buffer->num);
                   //  printk("the rxFull buffer status is not equals one\n");
                    rx_buffer=NULL;
                   
                }
            if(rx_buffer != NULL ) //judge the pid 
                if(rx_buffer->pid != current->tgid)
                {
                   // printk(KERN_INFO "the pid is not match!\n");
                    //printk(KERN_INFO "the rx_buffer->pid is %d, the current->pid is %d\n",rx_buffer->pid, current->tgid);
                    rx_buffer=NULL;
                }
                    
        }
  
        return rx_buffer; 
}        
        


struct tx_buffer_struct * find_tx_buffer(struct fpga_device * fpga, int flag){
        
        struct tx_buffer_struct * tx_buffer;
        tx_buffer=NULL;
        //printk(KERN_INFO "enter the find_tx_buffer function !");
        if(flag==0 ){
            tx_buffer=list_entry(fpga->txEmpty,struct tx_buffer_struct, list);  //get the buffer point
        }     
        else{
            tx_buffer=list_entry(fpga->txFull,struct tx_buffer_struct, list);  //get the buffer point
           // printk("the task buffer is to find is %d \n" ,tx_buffer->num);
        }
        
        if(tx_buffer->status != flag)  //it is the empty
                tx_buffer=NULL;
       
        return tx_buffer;
}

        

