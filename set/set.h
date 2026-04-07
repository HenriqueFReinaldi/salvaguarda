#ifndef HASH
#define HASH

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct SetItem{
    char* key;
    struct SetItem* next;
} SetItem;

typedef struct Set{
    size_t buckets;
    size_t elements;
    SetItem* SetItems;
} Set;



void initSet(Set* Set);
void addKey(Set* Set, char* key);
void removeKey(Set* Set, char* key);
int contains(Set Set, char* key);

#endif