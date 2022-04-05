#include "Queue.h"

int Queue[MAX_QUEUE];
int Front = 0;
int Rear = -1;
int ItemCount = 0;

int peek() {
   return Queue[Front];
}

bool isEmpty() {
   return ItemCount == 0;
}

bool isFull() {
   return ItemCount == MAX_QUEUE;
}

void insert(int data) {

   if(!isFull()) {
	
      if(Rear == MAX_QUEUE - 1) {
         Rear = -1;            
      }       

      Queue[++Rear] = data;
      ItemCount++;
   }
}

int removeData() {
   int data = Queue[Front++];
	
   if(Front == MAX_QUEUE) {
      Front = 0;
   }
	
   ItemCount--;
   return data;  
}