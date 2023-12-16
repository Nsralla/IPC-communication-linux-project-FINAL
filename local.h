#ifndef __LOCAL_H_
#define __LOCAL_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <wait.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/prctl.h>
#include <time.h>
#include <pthread.h>

/* This declaration is *MISSING* is many solaris environments.
   It should be in the <sys/sem.h> file but often is not! If 
   you receive a duplicate definition error message for semun
   then comment out the union declaration.
   */
#define MAX_ITEMS 10

// union semun {
//   int              val;
//   struct semid_ds *buf;
//   ushort          *array; 
// };

typedef struct {
  long msgtype;
  int customerId;
  int cashierId;
  int total;
  int sentBy;
  int flag;
} MESSAGEGUI;

struct MEMORY{
  int queueSize;
  int numberOfItems;
  int timeToScan;
  int behaviour;
}; 



typedef struct {
  char name[50];
  int quantity;
  float price;
}Item; // each Item in the file has there parameters

typedef struct {
  Item items[MAX_ITEMS];
  int itemCount;
}ShoppingCart; 



#define SEED   'g'		/* seed for ftok */
#define GUISEED 'a'
#define SERVER 1L
#define CLIENT 0L
#define SENTBYCASHIER 0
#define SENTBYCUSTOMER 1



typedef struct {
  long      msg_type;
  pid_t clientId; // client process ID not the index
  /* SHOPPING CART -> for now i'll assume it's just a number that represents how many items*/
  // I have removed int variable named items
  ShoppingCart cart;
} MESSAGE;


// Defining variables
// #define NUMBER_OF_SERVERS 2
// #define MINIMUM_ARRIVAL_RATE 10
// #define MAXIMUM_ARRIVAL_RATE 20

// #define MINIMUM_SHOPPING_TIME 10
// #define MANIMUM_SHOPPING_TIME 20

// #define MINIMUM_SCANNING_TIME 2
// #define MAXIMUM_SCANNING_TIME 5

// #define MAXIMUM_WAITING_TIME 50

// #define BEHAVIOUR_THRESHOLD 5

// #define ANGER_THRESHOLD 10

// #define INCOME_THRESHOLD 10000


int readThresholds(int thresholds[]);
int getRandom(int min, int max);

#endif
