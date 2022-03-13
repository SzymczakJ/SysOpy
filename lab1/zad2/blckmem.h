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
void delete_block_table(block_table * table);
int write_from_file_to_block(char * file_name, block_table * table);
void count_words(char * output_file_name, char * file_name, int name_length);
void remove_blocks(block_table * table, int block_index);
#endif //LAB1_LIBBLCKMEM_H
