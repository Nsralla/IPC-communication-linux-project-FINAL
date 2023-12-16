#include <raylib.h>
#include "local.h"

// typedef struct MESSAGEGUI{
//   long msgtype;
//   int customerId;
//   int cashierId;
//   int val;
//   int sentBy; // 0 = cashier, 1 = customer
//   int flag; //  0 = nothing , 1 = leaving 
// } MessageGui;

typedef struct Ents{
  int id;
  int total;
  int queue;
} Entity;

  Entity customers[100];
  Entity cashiers[100];
  int customerCounter = 0, cashierCounter = 0;
void* myThreadFunction(void* );
int main(void) {
  int screenWidth = 800;
  int screenHeight = 700;

  InitWindow(screenWidth, screenHeight, "Supermarket");
  


  // for (int i = 0 ; i < customerCounter ; i++){  // if we start wit hhigher than 0 for any reason. fill with dummy data
  //   Entity ent;
  //   ent.id = i;
  //   ent.queue = 5555;
  //   ent.total = -10;
  //   customers[i] = ent; 
  // }
  // for (int i = 0 ; i < cashierCounter ; i++){
  //   Entity ent;
  //   ent.id = -i;
  //   ent.queue = -5555;
  //   ent.total = -10;
  //   cashiers[i] = ent; 
  // }  

  Color rectangleColor = SKYBLUE;
  Color textColor = BLACK;

  Font font = LoadFontEx("Roboto-Black.ttf", 20, 0, 0); 

  // Define rectangle size
  int width = 100;
  int height = 50;

  SetTargetFPS(60);
    pthread_t myThread;
    if (pthread_create(&myThread, NULL, myThreadFunction, NULL) != 0) {
        return 1;
    }

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (customerCounter > 0){
        char **customerStrings = (char **)malloc(customerCounter * sizeof(char *));
        int baseX = 200;
        int baseY = 20;
        int stepSizeCustomer = screenHeight / customerCounter; // assuming 600 is constant & counter is always larger than 1. 
        for (int i = 0 ; i < customerCounter ; i++){
            customerStrings[i] = (char *)malloc(100);
            snprintf(customerStrings[i], 100, "id: %d, total: %d, queue: %d", customers[i].id, customers[i].total, customers[i].queue);
            //printf("2.Cashier string %s  cashier count = %d\n", customerStrings[i],cashierCounter);
            const char * text = customerStrings[i];
            DrawRectangle(baseX, ( baseY  + stepSizeCustomer*(i+1)) % screenHeight , width, height, rectangleColor);
            DrawTextEx(font, customerStrings[i], (Vector2){baseX , ( baseY  + stepSizeCustomer*(i+1)) % screenHeight}, font.baseSize, 0.0f, textColor);
        }
    }

    if(cashierCounter > 0){
        char **cashierStrings = (char **)malloc(cashierCounter * sizeof(char *));
        int baseCASHX = 600;
        int baseCASHY = 20;
        int stepSizeCashier = screenHeight / cashierCounter; // assuming 600 is constant & counter is always larger than 1. 
        for (int i = 0 ; i < cashierCounter ; i++){
            cashierStrings[i] = (char *)malloc(100);
            snprintf(cashierStrings[i], 100, "id: %d, total: %d, queue: %d", cashiers[i].id, cashiers[i].total, cashiers[i].queue);
            //printf("2.Cashier string %s  cashier count = %d\n", cashierStrings[i],cashierCounter);
            const char * text = cashierStrings[i];
            DrawRectangle(baseCASHX, ( baseCASHY + stepSizeCashier*(i+1) ) % screenHeight , width, height, rectangleColor);
            DrawTextEx(font, text, (Vector2){baseCASHX , ( baseCASHY + stepSizeCashier*(i+1)  ) % screenHeight }, font.baseSize, 0.0f, textColor);
        }
    }

    sleep(2);
    EndDrawing();
  }

  UnloadFont(font);
  CloseWindow();
  return 0;
}

void* myThreadFunction(void* args){
    key_t       key; 
    int mid;
    if ((key = ftok(".", GUISEED)) == -1) {    
      perror("GUI : key generation");
      return 1;
    }
    if ((mid = msgget(key, 0 )) == -1 ) {
      mid = msgget(key,IPC_CREAT | 0777);
    }

    printf("\nGUI: SUCCESSFULY CREATED Message Queue. Id =  %d \n", mid);

    while(1){

    MESSAGEGUI msg; 
    int n = msgrcv(mid, &msg, sizeof(msg), SERVER, 0);
    
    if (msg.sentBy == 0 & n != -1){
      // cahsier sent msg 
      if(msg.flag == 1){
        // removal!
        removeEntry(cashiers,100,msg.cashierId);
        cashierCounter--;
      }
      else if (msg.flag == 2){
        // means im updating the total for some customer! -> find customer based off id and update cost!
        for (int i = 0; i < 100; i++){
          if(customers[i].id == msg.customerId){
            customers[i].total = msg.total;
            break;
          }
        }
      }
      else if (msg.flag == 0){ // addition!
        printf("Added Cashier %d!\n", msg.cashierId);
        Entity ent;
        ent.id = msg.cashierId;
        ent.queue = msg.total;
        cashiers[cashierCounter] = ent;
        cashierCounter++;
      }
    }
    else if (msg.sentBy == 1 & n != -1){
      // customer sent msg 
      if(msg.flag == 1){
        // removal!
        removeEntry(customers,100,msg.customerId);
        customerCounter--;
      }
      else if (msg.flag == 0){
        printf("Added Customer %d!\n", msg.customerId);
        Entity ent;
        ent.id = msg.customerId;
        ent.queue = -1;
        customers[customerCounter] = ent;
        customerCounter++;
      }
      else if (msg.flag == 2){ // customer is setting his queue
        for (int i = 0; i < 100; i++){
          if(customers[i].id == msg.customerId){
            customers[i].queue = msg.total;
            break;
          }
        }
      }
    }
    }
}

void removeEntry(Entity arr[], int size, int entry) {
  int i;
  for (i = 0; i < size; ++i) {
    if (arr[i].id == entry) {
      break;
    }
  }

  if (i == size) {
    return; // Entry not found
  }

  for (int j = i + 1; j < size; ++j) {
    arr[j - 1] = arr[j];
  }
}