/********************************************
 * Filename: double_buffer.c
 * Description: using a double number and processes implements a pipeline
 *              that squares a number, adds 10 to it, and then takes the 
 *              square root of the result and prints value to console;
 * Author: Row 2
 * Date; 11/23/2026
 * compile: gcc -o double_buffer double_buffer.c -lpthread -lm
 *******************************************/

 #include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <sys/mman.h>
 #include <sys/wait.h>
 #include <semaphore.h>
 #include <unistd.h>
 #include <string.h>
 #include <math.h>

 #define SEM_NAME1 "/my_sem1"
 #define SEM_NAME2 "/my_sem2"
 #define SEM_NAME3 "/my_sem3"
 #define SEM_NAME4 "/my_sem4"
 #define ARR_SIZE 11

 int main(void)
 {

    //initialize semaphores and array
    sem_t *sem1, *sem2, *sem3, *sem4;
    pid_t pid1, pid2;
    double nums[ARR_SIZE];

    //initialize array for any array size
    for (int i = 0; i < ARR_SIZE; i++)
    {
        nums[i] = i;
    }
    
    //open semaphores
    //semaphores with initial value 0 show when buffer is ready to read
    //semaphores with initial value 2 show when buffer is ready to write
    //sem 1 ans 2 are for buffer 1, sem 3 and 4 are for buffer 2
    sem1 = sem_open(SEM_NAME1, O_CREAT, 0666, 0);
    sem2 = sem_open(SEM_NAME2, O_CREAT, 0666, 2);
    sem3 = sem_open(SEM_NAME3, O_CREAT, 0666, 0);
    sem4 = sem_open(SEM_NAME4, O_CREAT, 0666, 2);
    //checks for error creating semaphores
    if (sem1 == SEM_FAILED || sem2 == SEM_FAILED || sem3 == SEM_FAILED || sem4 == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    //create buffers
    double *buffer1 = mmap(NULL, sizeof(double) * 2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    double *buffer2 = mmap(NULL, sizeof(double) * 2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    //checks for error creating buffers
    if (buffer1 == MAP_FAILED || buffer2 == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    pid1 = fork();
    //checks for error creating first child process
    if (pid1 < 0)
    {
        perror("fork");
        exit(1);
    }
    if (pid1 == 0)
    {
        pid2 = fork();
        if (pid2 < 0)
        {
            perror("fork");
            exit(1);
        }
        //process responsible for squaring number
        if(pid2 == 0)
        {
            for (int i = 0; i < ARR_SIZE; i++)
            {
                sem_wait(sem2);
                sleep(1);
                double squared = nums[i] * nums[i];
                buffer1[i % 2] = squared;
                sem_post(sem1);
            }
            exit(0);
        }
        //process responsible for adding 10
        else
        {
            for (int i = 0; i < ARR_SIZE; i++)
            {
                sem_wait(sem4);
                sem_wait(sem1);
                sleep(1);
                double squared = buffer1[i % 2];
                sem_post(sem2);
                double added = squared + 10;
                buffer2[i % 2] = added;
                sem_post(sem3);
            }
            wait(NULL);
            exit(0);
        }
    }
    //proces responsible for taking sqrt
    else
    {
        for (int i = 0; i < ARR_SIZE; i++)
        {
            sem_wait(sem3);
            double added = buffer2[i % 2];
            sem_post(sem4);
            sleep(1);
            double result = sqrt(added);
            printf("Original number: %.2f, Result: %.4f\n", nums[i], result);
        }
        wait(NULL);
    }

    //clean up
    munmap(buffer1, sizeof(double) * 2);
    munmap(buffer2, sizeof(double) * 2);
    sem_unlink(SEM_NAME1);
    sem_unlink(SEM_NAME2);
    sem_unlink(SEM_NAME3);
    sem_unlink(SEM_NAME4);
    return 0;
 }
