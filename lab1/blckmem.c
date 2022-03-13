#include "libblckmem.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

block_table * create_block_table(number_of_blocks) {
    block_table * new_table = calloc(sizeof(memory_block), 1);
    new_table->number_of_blocks = number_of_blocks;
    new_table->blocks = calloc(sizeof(memory_block*), number_of_blocks);
    return new_table;
}

void remove_blocks(block_table * table, int block_index) {
    if (table[block_index] == 0) return;

    memory_block * block = table->blocks[block_index];
    free(block->wc_results);
    free(block)
}

void count_words(char * output_file_name, char * file_name, int name_length) {
    FILE * file = fopen(file_name, "w");
    char * command = calloc(sizeof(char), 4 + name_length);
    char * wc_res = calloc(sizeof(char), name_length + 100);

    sprintf(command, "wc %s",file_name);

    pf = popen(command,"r");

    fgets(wc_res, name_length + 100 , pf);
    int input_size = strlen(wc_res);
    FILE * output_file = fopen(output_file_name, "w");
    fwrite(wc_res, sizeof(char), input_size, output_file);

    fclose(file);
    fclose(output_file);

    free(command);
    free(wc_res);
}

int write_from_file_to_block(char * file_name, block_table * table) {
    int block_index = 0;
    for (; block_index < table->number_of_blocks; block_index++) {
        if (table[block_index] != 0) break;
    }

    if (block_index >= table->number_of_blocks) return -1;

    FILE * file = fopen(file_name, "r");
    int number_of_characters = 0;
    while (!feof(file)) {
        number_of_characters++;
    }
    fseek(file, 0, SEEK_SET);

    memory_block * block = calloc(sizeof(memory_block), 1);
    table->blocks[block_index] = block;
    block->wc_results = calloc(sizeof(char), number_of_characters);
    fgets(block->wc_results, number_of_characters, file);
    block->result_size = number_of_characters;
    fclose(file);

    return block_index;
}
