//
// Created by Jacopo Clocchiatti on 08/12/22.
//

#include "list.c"

typedef struct HashElement HashElement;
typedef struct HashBucket HashBucket;
typedef struct HashTable HashTable;

struct HashElement{
    char node_name;
    int position;
};

struct HashBucket{
    HashElement* arr;
    int len;
};

struct HashTable{
    HashBucket* buckets;
    int len;
};

// todo: decide hash function
int hash_func(){
    return 0;
}

HashTable* init_hashtable(int capacity){
    HashTable* hashTable = (HashTable*) calloc(1, sizeof(HashTable));
    if (!hashTable){
        fprintf(stderr, "Failed to create hash table");
    }
    hashTable->len = 0;
    hashTable->buckets = (HashBucket*) calloc(capacity, sizeof(HashBucket));
    if (!hashTable->buckets){
        fprintf(stderr, "Failed to create hash table buckets");
    }
    for (int i = 0; i < capacity; i++){
        hashTable->buckets->arr = (HashElement*) calloc(capacity, sizeof(HashElement));
        if (!hashTable->buckets->arr){
            fprintf(stderr, "Failed to create hash table bucket's list for bucket %d", i);
        }
    }
    return hashTable;
}

void free_hashtable(HashTable* hashTable){
    if (!hashTable){
        return;
    }

    for (int i = 0; i < hashTable->len; i++){
        free(hashTable->buckets[i].arr);
    }
    free(hashTable->buckets);
    free(hashTable);
}

void insert_into_hash(){
    // todo()
}

void delete_from_hash(){
    // todo()
}

void update_position(){
    // todo()
}

void find_element(){
    // todo()
}