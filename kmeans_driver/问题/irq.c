
//程序中涉及到中断的地方就下面两个地方 申请中断与中断处理函数

//申请中断
//第三个参数可以设置中断的上升沿触发，还是边沿触发 ，发现设置效果都一样
err = request_irq(fpga->pdev->irq, fpga_interrupt, IRQF_DISABLED,DEVICE_NAME, fpga);
        if (err) {
            printk(KERN_ERR "fpga: Unable to allocate interrupt "
                    "handler: %d\n", err);
        }else{
            printk(KERN_INFO "request_irq scuessfully!\n");
        }
        

        
//中断处理函数
static irqreturn_t fpga_interrupt(int irq, void *dev_id) //delete the parameter of  struct pt_regs *regs
{
    struct fpga_device *fpga;
    
    u32 status;
    
    fpga=(struct fpga_device *)dev_id;   //modify in this version 2015/06/15
  
    //read the interrupt status
    status= ioread32((void *)fpga->baseAddr + INTERRUPTSTATUS_REG);  
    printk(KERN_INFO "the value  of  status reg is %d", status);
    if(status==2) {  //DMA读完成 ，发送的中断 RAM-》 ＦＰＧＡ
        
            printk(KERN_INFO
            "the interrupt because of DMA read_complete\n");
            //更新缓冲区状态 
            //唤醒上层的写进程
             return IRQ_HANDLED;
        }else if(status ==1){ //DMA写完成 发出的中断 　FPGA->RAM 
            printk(KERN_INFO
            "the interrupt because of DMA write_complete\n ");
            //更新缓冲区状态 
            //唤醒上层的读进程
            return IRQ_HANDLED;
        }else{  //意料之外的中断
            printk(KERN_INFO "interrupt Error\n");
        
            return IRQ_HANDLED;  //
        }    
}
//现象是：在申请完中断之后就会有很多次的中断处理函数的调用，也就是上面的意料之外的调用
//在每次读完成或写完成发出中断时，会有一次读完成或写完成中断，还会附带着几次意料之外的中断

 
        
