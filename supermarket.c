#include "local.h"

 int currentBehaviour = 0, currentCustomerImpatient =0, NUMBER_OF_SERVERS, MINIMUM_ARRIVAL_RATE, MAXIMUM_ARRIVAL_RATE, ANGER_THRESHOLD, BEHAVIOUR_THRESHOLD;

void signal_catcher(int );
void cleanUp();
int main(int argc, char *argv[]){

    int thresholds[12];
    int count;
    count = readThresholds(thresholds);
    
    MINIMUM_ARRIVAL_RATE = thresholds[0];
    MAXIMUM_ARRIVAL_RATE = thresholds[1];
    BEHAVIOUR_THRESHOLD = thresholds[7];
    ANGER_THRESHOLD = thresholds[8];
    NUMBER_OF_SERVERS = thresholds[11];

    char buff[20];
    sprintf(buff, "%d", NUMBER_OF_SERVERS);

    for (int i = 0 ; i < NUMBER_OF_SERVERS ; i++){
        switch (fork()) {
            case -1:
            perror("Cashier: fork");
            return 2;

            case 0:
            char buffer[20];
            sprintf(buffer, "%d", i);          
            execlp("./cashier", "cashier", buffer, "&", 0);
            perror("Cashier: exec");
            return 3;
        }
    }

    if ( sigset(2, signal_catcher) == SIG_ERR ) { // behaviour
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }
    if ( sigset(10, signal_catcher) == SIG_ERR ) { // customers
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }
    if ( sigset(12, signal_catcher) == SIG_ERR ) { // income
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }

    int sleepTime = getRandom(MINIMUM_ARRIVAL_RATE,MAXIMUM_ARRIVAL_RATE);

    printf("SUPERMARKET: ARRIVAL RATE: %d\n", sleepTime);
    
    while(1){
        sleep(sleepTime);

        // Create Client
      
        switch (fork()) {
            case -1:
            perror("Client: fork");
            return 2;

            case 0:        
            execlp("./customer", "customer", buff, "&", 0);
            perror("customer: exec");
            return 3;
    
        }
      
    }
}

void signal_catcher(int the_sig){
  printf("\nSUPERMARKET: Signal %d received.\n", the_sig);

  switch(the_sig){
    case 2:
        currentBehaviour++;
        printf("SUPERMARKET: Current Behaviour Count Has Just Been Increased: %d\n", currentBehaviour);
        if(currentBehaviour < BEHAVIOUR_THRESHOLD){
            break;
        }
        cleanUp();
        exit(2);
    case 10:
        currentCustomerImpatient++;
        printf("SUPERMARKET: Current Impatience Count Has Just Been Increased: %d\n", currentCustomerImpatient);
        if(currentCustomerImpatient < ANGER_THRESHOLD){
            break;
        }
        cleanUp();
        exit(10);
    case 12:
        printf("Has Just Been Notified that a child has reached the income threshold!\n");
        cleanUp();
        exit(12);
  }
}

void cleanUp(){
    // MSG QUEUE -> ID => CHARACTER BASED ||||| SHMEM -> ID -> PPID + index
    printf("PARENT CLEANING UP!\n");
    struct MEMORY mem;
    int  pid = (int) getpid();
     for (int i = 0 ; i < NUMBER_OF_SERVERS ; i++){
        key_t key = ftok(".", SEED + i);
        int mid = msgget(key, 0);
        msgctl(mid, IPC_RMID, (struct msgid_ds *) 0); /* remove first message queue*/
        
        
        // int shmid = shmget(pid + i, sizeof(mem), 0); // POSSIBLE: CHANGE LAST ARG TO 0 
        // shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0);
     }
}
