#ifndef LAB1_LIBBLCKMEM_H
#define LAB1_LIBBLCKMEM_H
#include <stddef.h>
#include <stdio.h>

struct memory_block {
    size_t result_size;
    char * wc_results;
};

struct block_table {
    size_t number_of_blocks;
    memory_block ** blocks;
};

void show_pwd();
block_table * create_block_table(int number_of_blocks);
void delete_block_table(block_table * table);
int write_from_file_to_block(FILE * file_name, block_table * table);
void count_words(FILE * output_file_name, char * file_name, int name_length);
void remove_blocks(block_table * table, int block_index);
void print_all_blocks(block_table * table);
#endif //LAB1_LIBBLCKMEM_H
