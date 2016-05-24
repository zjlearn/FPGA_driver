//这个文件是利用了C语言的多线程机制，然后对FPGA设备进行读写操作
//使用两个线程分别对其进行读写操作。

//具体的C语言多线程操作中的知识可以参考：
/*
http://blog.csdn.net/koches/article/details/76244
http://blog.csdn.net/koches/article/details/7624400

使用
int   pthread_create(pthread_t   *   thread, pthread_attr_t * attr, void * (*start_routine)(void *), void * arg)
来进行线程的创建。


thread_join使一个线程等待另一个线程结束。
代码中如果没有pthread_join主线程会很快结束从而使整个进程结束，从而使创建的线程没有机会开始执行就结束了。加入pthread_join后，主线程会一直等待直到等待的线程结束自己才结束，使创建的线程有机会执行。
所有线程都有一个线程号，也就是Thread ID。其类型为pthread_t。通过调用pthread_self()函数可以获得自身的线程号。

在编译中要加 -lpthread参数
    gcc thread.c -o thread -lpthread
    thread.c为你些的源文件，不要忘了加上头文件#include<pthread.h>


*/

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
    FILE * fpRead=((THREAD_PARAM *)m)->_fp;
    int fd =((THREAD_PARAM *)m)->_device;
    char inbuf[4160032];
    //get result from  data into the inbuffer
    for(i=0; i<4160032; i++){
       fscanf(fpRead,"%d",&inbuf[i]);
       //printf("%02x",inbuf[i]);
    }
    for (i=0; i<32 ; i++)
        printf("%02x",inbuf[i]);
  for (i=0; i<NUM; i++){
      len=write(fd,inbuf,4160032);
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
    unsigned char  outbuf[545];
    for(i=0; i<NUM; i++){
     len=read(fd,outbuf,544); //the 234 is 
      if(len<0)
        { printf("Error redaing from the device!\n");
          close(fd);
          exit(1);
        } 
        printf("reading  %d bytes from the device!\n",len);
  }
   for(i=0; i<544; i++)
       printf("%02x",outbuf[i]);
  //write the result into the fileleng
  //fputs(outbuf,fpWrite);
   for(i=0; i<544; i++)
       fprintf(fpWrite,"%02x",outbuf[i]);
}

int main()
{
    void print_msg(void*);
    int fd;
    fd=open("/dev/reducefpga1",O_RDWR);
    pthread_t t1,t2;
    
    FILE  *fp_result=fopen("result.txt","w");
    FILE *fp_data=fopen("4M_data.txt","r");
    
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


