#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>           
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <math.h>


#define SEM_NAME1 "/wait_process1"
#define SEM_NAME2 "/wait_process2"


int main(){
    pid_t pid;
    sem_t *p1 = sem_open(SEM_NAME1, O_CREAT, 0666,1);
    sem_t *p2 = sem_open(SEM_NAME2, O_CREAT, 0666,1);

    double *buffer1 = mmap(NULL,sizeof(double),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
    double *buffer2 = mmap(NULL,sizeof(double),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
    double *buffer_output = mmap(NULL,sizeof(double),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
    
    //regular values for the shared memory
    *buffer1 = -100;
    *buffer2 = -100;
    *buffer_output = -100;
    
    int count1 = 0;     //controls output index
    int count2 = 0;     //controls input index
    float x = 0;


    float input[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    float output[11];

   while(count1<11){
    pid = fork();

    if(pid == 0){
        pid_t pid2 = fork();
        //adds ten
        if(pid2 == 0){
            sleep(1);
            sem_wait(p1);
            double read = *buffer1;
            sem_post(p1);
            if(read == -100){
                exit(1);
            }
            read = read + 10;
            sem_wait(p2);
            *buffer2 = read;
            sem_post(p2);

            exit(1);
        }
        //square roots
        else{
            sleep(1);
            float read = 0;
            sem_wait(p2);
            read = *buffer2;
            sem_post(p2);
            if(read == -100){
                waitpid(pid2, NULL,0);
                exit(1);
            }
            read = sqrt(read);
            *buffer_output = read;

            waitpid(pid2,NULL,0);
            exit(1);

        }
    }
    //squares
    else{
        sleep(1);
        if(count2 >10){
            x = -100;
        }
        else{
        x = input[count2];
        count2++;
        x = x*x;
        }
        sem_wait(p1);
        *buffer1 = x;
        sem_post(p1);
        waitpid(pid,NULL,0);
    }

    if(*buffer_output == -100){
    }
    else{
    output[count1] = *buffer_output;
    count1++;
    }
}
for(int i = 0; i < 11; i++){
    printf("%f transformed into: %f\n",input[i],output[i]);
}
    return 0;
}