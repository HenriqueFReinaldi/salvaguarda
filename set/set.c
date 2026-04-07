#include "set.h"

void initSet(Set* Set){
    (Set->SetItems) = malloc((Set->buckets)*sizeof(SetItem));
    for (size_t i = 0; i < Set->buckets; i++){
        Set->SetItems[i].key = NULL;
        Set->SetItems[i].next = NULL;
    }

}

size_t getBucket(char* string, size_t buckets){
    unsigned long hash = 5381;

    int c;
    while ((c = *string++)) hash = ((hash << 5) + hash) + c;

    unsigned long bucket = hash % buckets;
    return (size_t)bucket;
}


void addKey(Set* Set, char* key){
    size_t key_bucket = getBucket(key, Set->buckets);
    SetItem* item = &(Set->SetItems[key_bucket]);

    while (item->next != NULL){
        if (strcmp(item->key, key) == 0) return;
        item = item->next;
    }

    if(item->key == NULL){
        item->key = key;
        SetItem* new_item = malloc(sizeof(SetItem));
        *new_item = (SetItem){NULL, NULL};
        item->next = new_item;
        Set->elements++;
        return;
    }
}

void removeKey(Set* Set, char* key){
    size_t key_bucket = getBucket(key, Set->buckets);
    SetItem* item = &(Set->SetItems[key_bucket]);
    SetItem* last_item = item;

    while (item->next != NULL){
        if (strcmp(item->key, key) == 0){
            Set->elements--;
            if (item == last_item){
                Set->SetItems[key_bucket] = *item->next;
                return;
            }
            last_item->next = item->next;
            free(item);
            return;
        }
        last_item = item;
        item = item->next;
    }
}

int contains(Set Set, char* key){
    size_t key_bucket = getBucket(key, Set.buckets);
    SetItem* item = &(Set.SetItems[key_bucket]);

    while (item->next != NULL){
        if (strcmp(item->key, key) == 0){
            return 1;
        }
        item = item->next;
    }
    return 0;
}