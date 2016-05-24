#ifndef BUFFER_H

#define BUFFER_H

#include<linux/list.h>

#include "FPGA.h"


#define TX_BUFFER_SIZED PAGE_SIZE*1024  //4M Byte
#define RX_BUFFER_SIZE   PAGE_SIZE*512   //2M Byte 
#define NUM_RX_BUFFS   10   //the number of RX buffer  
#define NUM_TX_BUFFS   10   //the number of tx buffer




#endif
//the statement


