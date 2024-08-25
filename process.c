#include <stdio.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>


struct message_type_1
{
    long type;
    int pid;
} message_type_1;

struct message_type_2
{
    long type;
    int pageorframe;
    int pid;
} message_type_2;

#define Wait(s) semop(s, &waitop, 1)
#define Signal(s) semop(s, &signalop, 1)


int main(int argc, char *argv[])
{
    char refstr[1000];
    int i = 0;
    strcpy(refstr, argv[1]);
    printf("\033[1;31m");
    printf("\nProcess started with Refernce String %s\n", refstr);
    printf("\033[0m");


    int pid = getpid();

    struct message_type_1 message1;
    message1.pid = pid;
    message1.type = 1;


    int msg1_shm = atoi(argv[2]);
    int msg3_shm = atoi(argv[3]);
    msgsnd(msg1_shm, (void *)&message1, sizeof(message_type_1), 0);

    struct sembuf waitop = {0, -1, 0};
    struct sembuf signalop = {0, 1, 0};

    
    struct message_type_1 msgr;
    int previ=0;;

    msgrcv(msg1_shm,(void*)&msgr,sizeof(struct message_type_1),pid,0);

    while (refstr[i] != '\0')
    {
        int j = 0;
        previ=i;
        while (refstr[i] != '.' && refstr[i] != '\0')
        {   
            j *=10;
            j+= (refstr[i] - '0');
            i++;
        }

        i++;
        printf("\033[0m\nPID %d:Page Number retrieved from string \033[1;34m = %d\n\033[0m", pid, j);

        struct message_type_2 message2;
        message2.pid = pid;
        message2.type = 1;
        message2.pageorframe = j;
        msgsnd(msg3_shm, (void *)&message2, sizeof(message_type_2), 0);
        msgrcv(msg3_shm, (void *)&message2, sizeof(message_type_2), pid, 0);
        
        if (message2.pageorframe == -1)
        {
            printf("PID %d:Page Fault Detected \nPutting Process on wait until page is loaded\n",pid);
            msgrcv(msg1_shm,(void*)&msgr,sizeof(struct message_type_1),pid,0);
            i=previ;
            continue;
        }
        else if (message2.pageorframe == -2)
        {
            printf("PID %d:Page Illegal detected \nTerminating Process \n\n",pid);
            exit(1);
        }
        else
        {
            printf("PID %d:Frame Allocation done, Frame number alloted: %d\n",pid, message2.pageorframe);
        }
    }

    struct message_type_2 message2;
    message2.type = 1;
    message2.pageorframe = -9;
    message2.pid = pid;

    printf("PID %d:", pid);

    msgsnd(msg3_shm, (void *)&message2, sizeof(message_type_2), 0);
    printf("All Frames Received \n");
    printf("Process Terminating\n\n");

    return 0;
}
