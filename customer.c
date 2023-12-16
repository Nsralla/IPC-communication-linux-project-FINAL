#include "local.h"

    int MINIMUM_SHOPPING_TIME, MAXIMUM_WAITING_TIME;
    int MAXIMUM_SHOPPING_TIME;

    int iamAlive = 0;

int readSuperMarketData(Item items[], int *itemCount){
    FILE* file = fopen("data.txt","r"); //open file

    if (file == NULL) {
        printf("CUSTOMER: ERROR OPENING THE FILE\n");
        return 0;
    }
    *itemCount = 0;
    while(fscanf(file, "%s %d %f", items[*itemCount].name,&items[*itemCount].quantity,&items[*itemCount].price) == 3){
        (*itemCount)++;
        if(*itemCount >= MAX_ITEMS){
            printf("CUSTOMER: TOO many items in the file.\n");
            break;
        }
    }
    fclose(file);
    return 1;
}

void printItemsInCart(ShoppingCart *cart){
    for(int i = 0;i<cart->itemCount; i++){
        printf("%s\n", cart->items[i].name);
    }

}

void addToCart(ShoppingCart *cart, Item*item){
    if(cart->itemCount < MAX_ITEMS){
        cart->items[cart->itemCount] = *item;
        cart->itemCount++;
    }
    else{
        printf("CUSTOMER: The cart is full.\n");
    }
}

void simulateShopping(ShoppingCart *cart,Item items[], int itemCount ){
    srand(time(NULL));
    int shoppingTime = getRandom(MINIMUM_SHOPPING_TIME,MAXIMUM_SHOPPING_TIME);
    int itemsCounter = getRandom(1,MAX_ITEMS);
    printf("CUSTOMER: ID = %d has started shopping for : %d\n",(int)getpid(),shoppingTime);
    for(int i = 0; i < itemsCounter; i++){
        int randomItemIndedx = rand() % itemCount;
        if(items[randomItemIndedx].quantity > 0){
            items[randomItemIndedx].quantity--;
            addToCart(cart, &items[randomItemIndedx]);
            printf("CUSTOMER {%d}: Added %s to the cart.\n", getpid(),items[randomItemIndedx].name);
        }
        else{
            printf("CUSTOMER: %s is out of stock.\n", items[randomItemIndedx].name);
        }
    }
    sleep(shoppingTime);
    printf("CUSTOMER: ID = %d Has just finished shopping! \n", (int)getpid());
}

int bestCashier(int cashiersNumber,int weights[]){
    pid_t ppid = getppid();
    if (ppid == -1){
         perror("CUSTOMER: Error getting parent id\n");
        exit(EXIT_FAILURE);
    }

    struct MEMORY * memory[cashiersNumber]; 
    char          *shmptr;
    for(int i =0 ; i < cashiersNumber; i++){

        int shmId = shmget(((int)getppid() + i), 0, 0);
        if(shmId == -1){
            printf("IN loop iteration {%d} i failed to connect to shmem %d!\n", i, ((int)getppid() + i));
            perror("CUSTOMER: Error connecting to shared memory");
            exit(EXIT_FAILURE);
        }
        if ( (shmptr = (struct MEMORY *) shmat(shmId, NULL, 0)) == (char *) -1 ) {
            perror("shmptr -- parent -- attach");
            exit(1);
        }
        printf("Customer {%d} reading shmem of [%d] \n", (int)getpid() , i );
        memory[i] = (struct MEMORY *)shmptr;
    }
    int evaluation[cashiersNumber]; 
    for(int i =0; i<cashiersNumber;i++){
        evaluation[i] = memory[i]->queueSize*weights[0] + memory[i]->numberOfItems * weights[1] + memory[i]->timeToScan * weights[2] + memory[i]->behaviour*weights[3];
    }
    int max = evaluation[0], index = 0;
    for(int i=0;i<cashiersNumber;i++){
        if(max < evaluation[i]){
            max=evaluation[i];
            index = i;
        }
    }

    connectTOGUIQueue(2, index);
    printf("CUSTOMER{%d}: I have decided to go with cashier index = [%d] \n", (int)getpid(), index);
    return index;
}

void leaveQueue(int signum){
    if(iamAlive == 0){ //
            kill(getppid(), SIGUSR1);
            printf("CUSTOMER {%d}: Can't wait in the queue %d\n", getpid(),getpid());
            sleep(5);
            connectTOGUIQueue(1, -1);//connect to gui queue, pass 1 (leaving)
            exit(EXIT_FAILURE);
        }
      else{
        // mayble add another connection to GUI queue also here
        printf("customer {%d} ALARM HAS RING, BUT IAM NOT LEAVING SINCE  ITS MY TURN\n",getpid());
      }  
    }


void connectTOGUIQueue(int flag, int total){
    __key_t key2 = ftok(".",GUISEED);
     if (key2 == -1){
        perror("CUSTOMER: Error creating key  GUI QUEUE.\n");
        exit(EXIT_FAILURE);
    }

    int msgid2 = msgget(key2, 0);
    if (msgid2 == -1){
         perror("CUSTOMER: Error making the message queue for the GUI\n");
        exit(EXIT_FAILURE);
    }

    MESSAGEGUI guiMessage;
    guiMessage.customerId =(int) getpid();
    guiMessage.cashierId = 0; 
    guiMessage.flag = flag;
    guiMessage.msgtype = SERVER;
    guiMessage.sentBy = 1;
    guiMessage.total = total;

    int error = msgsnd(msgid2,&guiMessage,sizeof(guiMessage),0);
    if(error == -1){
        perror("CUSTOMER: Error sending the message to the GUI queue\n");
        exit(EXIT_FAILURE);
    }
    printf("Customer {%d} connected to GUI!\n", (int)getpid());
    sleep(1);
}


// here check 'SEED', and index-1 (since I have started from 1 not 0)
void connect_to_the_message_queue(int index, ShoppingCart cart){
    // create key
    __key_t key = ftok(".", SEED + index );
    if (key == -1){
        perror("CUSTOMER: Error creating key\n");
        exit(EXIT_FAILURE);
    }

    int msgid = msgget(key, 0); // get msg queue id
    if (msgid == -1){
         perror("CUSTOMER: Error making the message queue\n");
        exit(EXIT_FAILURE);
    }

    // create a message
    //printf("CUSTOMER: Id = %d CONNECTED TO MSGQID = %d\n", getpid(), msgid);
    MESSAGE msg;
    msg.msg_type = SERVER;
    msg.cart = cart;
    msg.clientId = getpid(); // get customer process ID


    // send the message to the cashier
    int err = msgsnd(msgid, &msg, sizeof(msg), 0);
    if(err == -1){
         perror("CUSTOMER: Error sending the message to the cashier queue\n");
        exit(EXIT_FAILURE);
    }

    if(sigset(SIGALRM, leaveQueue)){ // to handle the alarm signal
        perror("Sigset can not set SIGALRM");
        exit(SIGALRM);
    }
    //printf("CUSTOMER: Id = %d has just sent a message and now im sleeping !\n", getpid());
    alarm(MAXIMUM_WAITING_TIME); 
   
    while(1){
        pause();
    }
}

    

    void stillAlive(int signum){
        iamAlive = 1; // don't leave the supermarket
        printf("CUSTOMER {%d} yes I am still availible\n\n",getpid());
        while(1){
            pause();
        }
    }

    void recieveCashierMessage(int signum){
        sleep(5);
        connectTOGUIQueue(1, -1);
        printf("CUSTOMER: Customer %d has finished.\n", getpid());
        exit(EXIT_SUCCESS);
    }


int main(int args, char*argv[]){
    
    connectTOGUIQueue(0, -1);

    int thresholds[12];
    int count;
    count = readThresholds(thresholds);
    MAXIMUM_SHOPPING_TIME = thresholds[3];
    MINIMUM_SHOPPING_TIME = thresholds[2];
    MAXIMUM_WAITING_TIME = thresholds[6];


    prctl(PR_SET_PDEATHSIG, SIGHUP); // GET A SIGNAL WHEN PARENT IS KILLED
    if(sigset(SIGUSR1, stillAlive) == -1){ 
        perror("Sigset can not set SIGUSR1");
        exit(SIGUSR1);
    }
    if(sigset(SIGUSR2, recieveCashierMessage) == -1){
        perror("Sigset can not set SIGUSR2");
        exit(SIGUSR2);
    }
    int numberOfCashier = 0; 
    if (args < 2){
        perror("CUSTOMER: Number of args is less than 2\n");
        exit(EXIT_FAILURE);
    }
    else{
        numberOfCashier = atoi(argv[1]);
    }

    Item items[MAX_ITEMS]; 
    int itemCount;
    if(!readSuperMarketData(items, &itemCount)){
        return -1;
    }

    srand(time(NULL));
    printf("CUSTOMER: {%d} has started shopping\n",getpid());
    ShoppingCart cart;
    cart.itemCount = 0;
    simulateShopping(&cart, items, itemCount);

    int wieghts[4];
    wieghts[0] = -1;
    wieghts[1] = -1;
    wieghts[2] = -2;
    wieghts[3] = 1;
    int best_cashier_index = bestCashier(numberOfCashier, wieghts);

    connect_to_the_message_queue(best_cashier_index, cart);
    return 0;
}
