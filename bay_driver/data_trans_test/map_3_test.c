
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<error.h>
#include<fcntl.h>
#include<sys/types.h>
#include<string.h>



#define NUM 1

typedef struct{
    FILE *_fp;
    int _device;
}THREAD_PARAM;


void * sendTask(void *m){
    int i;
    int len;
    
    int fd =((THREAD_PARAM *)m)->_device;
    
    FILE * fpRead=((THREAD_PARAM *)m)->_fp;
    
    char inbuf[4160032];
    //get result from  data into the inbuffer
    for(i=0; i<4160032; i++){
       fscanf(fpRead,"%d",&inbuf[i]);
       //printf("%02x",inbuf[i]);
    }
    for (i=0; i<32 ; i++)
        printf("%02x",inbuf[i]);
  for (i=0; i<NUM; i++){
      len=write(fd,inbuf,0);
      if(len<0)
        { printf("Error writing to the device!\n");
          close(fd);
          exit(1);
        } 
        printf("writing %d bytes to the device!\n",len);
    }
}

void * getResult(void *m ){
    int i;
    int len;
    FILE *fpWrite=((THREAD_PARAM *)m)->_fp;
    int fd=((THREAD_PARAM *)m)->_device;
    unsigned char  outbuf[1632];
    for(i=0; i<NUM; i++){
     len=read(fd,outbuf,1632); //the 234 is 
      if(len<0)
        { printf("Error redaing from the device!\n");
          close(fd);
          exit(1);
        } 
        printf("reading  %d bytes from the device!\n",len);
  }
   for(i=0; i<1632; i++)
       printf("%02x",outbuf[i]);
  //write the result into the file
  for(i=0; i<43520; i++)
       fprintf(fpWrite,"%02x",outbuf[i]);

}

int main()
{
    void print_msg(void*);
    int fd;
    fd=open("/dev/mapfpga1",O_RDWR);
    pthread_t t1,t2;
    
    FILE  *fp_result=fopen("map_key3.txt","w");
    FILE *fp_data=NULL;
    
    THREAD_PARAM param1;
    memset(&param1,0,sizeof(THREAD_PARAM));  
        param1._fp = fp_result;  
        param1._device=fd;
    pthread_create(&t2,NULL,getResult,&param1);
    
    THREAD_PARAM param2;
    memset(&param2,0,sizeof(THREAD_PARAM));  
        param2._fp = fp_data;  
        param2._device=fd;
    pthread_create(&t1,NULL,sendTask,&param2);

    pthread_join(t1,NULL);
    pthread_join(t2,NULL); 
    close(fd);
    fclose(fp_result);
    fclose(fp_data);
}
