#ifndef LAB1_LIBBLCKMEM_H
#define LAB1_LIBBLCKMEM_H
#include <stddef.h>
#include <stdio.h>

typedef struct {
    size_t result_size;
    char * wc_results;
}memory_block;

typedef struct {
    size_t number_of_blocks;
    memory_block ** blocks;
}block_table;

block_table * create_block_table(number_of_blocks);

#endif //LAB1_LIBBLCKMEM_H
