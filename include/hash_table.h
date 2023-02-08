//
// Created by Jacopo Clocchiatti on 07/12/22.
//

#ifndef HASHTABLE_H
#define HASHTABLE_H

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

#endif