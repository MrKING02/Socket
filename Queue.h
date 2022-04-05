#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

#define MAX_QUEUE 10

int peek();
bool isEmpty();
bool isFull();
void insert(int);
int removeData();

#endif