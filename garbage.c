#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

void** garbageBin;
int length;
int size;

void** ensureCapacity(void** array) {
	void** newArray = (void**)malloc(sizeof(void**) * size*2);
	//addToGarbage(newArray);
	for(int i = 0; i < size; i++) {
		newArray[i] = array[i];
	}
	free(array);
	garbageBin = newArray;
	size = size*2;
	return garbageBin;
}


void initGarbageBin() {
	size = 10;
	length = 0;
	garbageBin = (void**)malloc(sizeof(void**) * size);
	
	}

void addToGarbage(void* garbage) {
	if(length >= size) {
		garbageBin = ensureCapacity(garbageBin);
	}
	garbageBin[length] = garbage;
	length ++;
}

void cleanGarbage() {
	for(int i=0; i < size; i++) {
		free(garbageBin[i]);
	}
	free(garbageBin);
}




