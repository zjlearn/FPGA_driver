#这个是针对MSIX中断方式进行修改的代码

#define  MSIX_NUM 5

struct msix_entey * ms_entey


ms_entey=kmalloc(sizeof(struct msix_entey *)*MSIX_NUM)

for(int i=0; i< MSIX_NUM; i++){
    ms_entey[i].entry=i+15;
}

pci_enable_msix(fpga->pdev,fpga->ms_entey, 4);




pci_disable_msix




















