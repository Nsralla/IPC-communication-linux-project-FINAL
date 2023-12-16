#include "local.h"

int getRandom(int,int);
void connectToGUIQueue(int ,int , int );
void handler(int );
int shmid;
char          *shmptr; // pointer to shared memory 
int main(int argc, char *argv[]){
  key_t       key; // key for generating msg queue
  pid_t       parent_pid = getppid(); // parent pid for defining the shared memory key
  int         mid, n; // msg queue id, n for reading ( will store char count) , shmid (shared memory id)
  MESSAGE     msg; // instance of message struct 

  struct  MEMORY* memory; // memory struct
  
  // union semun    arg; // arg for later use
  struct msqid_ds buf; // to get info on msg queue

  srand((unsigned) getpid());

  int thresholds[12];
  int count;
  count = readThresholds(thresholds);
  int MINIMUM_SCANNING_TIME = thresholds[4];
  int MAXIMUM_SCANNING_TIME = thresholds[5];
  int INCOME_THRESHOLD = thresholds[9];

  int behaviour = 100;
  int timeToScan = getRandom(MINIMUM_SCANNING_TIME, MAXIMUM_SCANNING_TIME); 
  int sales = 0;

  prctl(PR_SET_PDEATHSIG, SIGHUP); 
  sigset(SIGHUP,handler);
  

  if(argc < 2){
    perror("CASHIER: Not enough args");
    exit(-2);
  }

  int index = atoi(argv[1]);
  printf("\nCASHIER: ID = %d  Getting Started, Parent Pid: %d, Index: %d\n", (int)getpid(), parent_pid, index);
  
  if ((key = ftok(".", SEED + index)) == -1) {    
    perror("CASHIER:  Client: key generation");
    return 1;
  }

  if ((mid = msgget(key, 0 )) == -1 ) {
    mid = msgget(key,IPC_CREAT | 0777);
  }
  printf("\nCASHIER: SUCCESSFULY CREATED Message Queue. Id =  %d \n", mid);

  
    if ( (shmid = shmget(((int)parent_pid + index), sizeof(memory),
		       IPC_CREAT | 0777)) != -1 ) {
        
        if ( (shmptr = (struct MEMORY *) shmat(shmid, NULL, 0)) == (char *) -1 ) {
          perror("shmptr -- parent -- attach");
          exit(1);
        }
        //memcpy(shmptr, (struct MEMORY *) &memory, sizeof(memory));
        memory = (struct MEMORY *) shmptr;
        printf("CASHIER: SUCCESSFULY CREATED Shared Memory. Id =  %d\n", shmid);
  }
  else {
    perror("shmid -- parent -- creation");
    exit(2);
  }

  connectToGUIQueue(0,index, 0);

  int totalCost = 0 ;

  while(1){
    totalCost = 0;

    msgctl(mid, IPC_STAT, &buf);
    //printf("CASHIER: Current # of bytes on queue\t %d\n", buf.__msg_cbytes);
    //printf("CASHIER: Current # of messages on queue\t %d\n", buf.msg_qnum); /* Read Queue Status to update Shared Memory*/
    //printf("CASHIER: Time to scan = %d\n", timeToScan);
    //printf("CASHIER: Behaviour = %d\n", behaviour);

    memory->queueSize = buf.msg_qnum;
    memory->numberOfItems = buf.__msg_cbytes; // MUST DO EQUATION TO CALCULATE NUMBER OF ITEMS BASED ON SIZE -> AFTER DEFININE THE SHOPPING CART STRUCT
    memory->timeToScan = timeToScan;
    memory->behaviour = behaviour; /* Update Shared Memory */

    if ((n = msgrcv(mid, &msg, sizeof(msg), SERVER, 0)) == -1 ) { /* Start waiting for a message to appear in MQ */
      perror("CASHIER:  msgrcv error");
      return 2;
    }
    printf("CASHIER: Just recieved message by : %d\nCASHIER: Now testing to see if he's still available!\n", (int)msg.clientId);
    //printf("CASHIER: Number Of Items In the cart : %d\n",msg.cart.itemCount);    
    /* Handle the message (shopping cart) & calculate the total cost */
    

    /* start by checking if the process is still alive */

    pid_t c_pid = msg.clientId; // REAL PID FROM MSG
    if (kill(c_pid,SIGUSR1) == 0 ){

      /* Message still up -> handle it */
      //printf("CASHIER: Have just verified Customer(%d) is still here.\n CASHIER:Items in the cart:\n", (int)msg.clientId);
      for(int i = 0; i<msg.cart.itemCount;i++) // something wrong about msg.cart.itemCount
      {
        //printf("CASHIER: {%d} %s {Quantity: %d, price: %.2f}\n\n", index, msg.cart.items[i].name, msg.cart.items[i].quantity, msg.cart.items[i].price);
        //printf("item price:\n", msg.cart.items[i].price);
        totalCost += msg.cart.items[i].price; //increase the total coast
        sleep(timeToScan); // delay between priniting each item (scaning time) (change it and put cashier speed)
      } 
       // CONNECT TO GUI QUEUE
      connectToGUIQueue(2,totalCost,(int)msg.clientId);
      // print the total coast
      printf("CASHIER: index = {%d} Finished processing customer(%d) His total comes up to %d\n", index, (int) msg.clientId,totalCost); // regarding index is the id (from the loop) of the cashier
    }


    //printf("CASHIER:  informing customer(%d) that I'm done!\n", (int)msg.clientId);
    kill(c_pid,SIGUSR2); 

    /* Behaviour will decrease with time, total sales will be increased aswell. Check if conditions met & send signals if so.*/
    behaviour--;
    if (behaviour == 0){
      // CONNECT TO GUI QUEUE TO EXIT AND DISPLAY IN THE SCREEN
      connectToGUIQueue(1,0,0);
      kill(getppid(), 2); 
    }

    sales+=totalCost;
    if (sales >= INCOME_THRESHOLD){
      // CONNECT TO GUI QUEUE TO EXIT AND DISPLAY IN THE SCREEN
      connectToGUIQueue(1,0,0);
      kill(getppid(), 12);
    }
  }
  return 0;
}
void connectToGUIQueue(int flag,int total, int customerId){
    __key_t key2 = ftok(".",GUISEED);
     if (key2 == -1){
        perror("CASHIER: Error c'h'reating key for the GUI QUEUE.\n");
        exit(EXIT_FAILURE);
    }

    int msgid2 = msgget(key2, 0); 
    if (msgid2 == -1){
         perror("CASHIER: Error making the message queue for the GUI\n");
        exit(EXIT_FAILURE);
    }
    //create the message
    MESSAGEGUI guiMessage;
    guiMessage.customerId = customerId; 
    guiMessage.cashierId = (int) getpid(); 
    guiMessage.flag = flag;
    guiMessage.msgtype = SERVER; 
    guiMessage.sentBy = SENTBYCASHIER;
    guiMessage.total = total;

    // send the message
    int error = msgsnd(msgid2,&guiMessage,sizeof(guiMessage),0);
    if(error == -1){
        perror("CASHIER: Error sending the message to the GUI queue\n");
        exit(EXIT_FAILURE);
    }
}
void handler(int interruptNumber){
	// add logic of detaching and removing shmem
   if(shmdt(shmptr) == -1){
    perror("SHMDT");
    exit(EXIT_FAILURE);

   }
   int  pid = (int) getpid();
   shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0);
	exit(1);
}	
