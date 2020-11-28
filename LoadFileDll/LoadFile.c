//
// Created by Алексей on 27.11.2020.
//

#include "LoadFile.h"
#include <stdio.h>
#include <malloc.h>

char *LoadFile(const char *fileName) {
    FILE* filePtr = fopen(fileName, "rb");
    if(filePtr == NULL) return 0;
    fseek(filePtr, 0, SEEK_END);
    long size = ftell(filePtr) + 1;
    fseek(filePtr, 0, SEEK_SET);
    char* data = malloc(sizeof(char) * size);
    if(data != NULL){
        fread(data, sizeof(char), size - 1, filePtr);
        data[size - 1] = '\n';
    }
    return data;
}
