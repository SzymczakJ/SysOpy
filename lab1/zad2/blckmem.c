#include "blckmem.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>


block_table * create_block_table(int number_of_blocks) {
    block_table * new_table = calloc(sizeof(memory_block), 1);
    new_table->number_of_blocks = number_of_blocks;
    new_table->blocks = calloc(sizeof(memory_block*), number_of_blocks);
    return new_table;
}

void remove_blocks(block_table * table, int block_index) {
    if (table->blocks[block_index] == 0) return;

    memory_block * block = table->blocks[block_index];
    free(block->wc_results);
    free(block);
    table->blocks[block_index] = 0;
}

void count_words(FILE * output_file, char * file_names, int names_length) {
    char * command = calloc(sizeof(char), 4 + names_length);
    char * wc_res = calloc(sizeof(char), names_length + 500);
    sprintf(command, "wc %s",file_names);

    FILE * pf = popen(command,"r");

    int i = 0;
    while (!feof(pf)) {
        wc_res[i] = getc(pf);
        i++;
    }
    wc_res[i++] = '\0';
    int input_size = strlen(wc_res);
    fwrite(wc_res, sizeof(char), input_size, output_file);

    free(command);
    free(wc_res);
}

int write_from_file_to_block(FILE * file, block_table * table) {
    int block_index = 0;
    for (; block_index < table->number_of_blocks; block_index++) {
        if (table->blocks[block_index] == 0) break;
    }

    if (block_index >= table->number_of_blocks) return -1;

    fseek(file, 0, SEEK_END);
    int number_of_characters = ftell(file);
    fseek(file, 0, SEEK_SET);

    memory_block * block = calloc(sizeof(memory_block), 1);
    table->blocks[block_index] = block;
    block->wc_results = calloc(sizeof(char), number_of_characters + 1);
    int i = 0;
    while (i < number_of_characters) {
        block->wc_results[i] = getc(file);
        i++;
    }
    block->wc_results[i++] = '\0';
    block->result_size = number_of_characters;
    fclose(file);

    return block_index;
}

void delete_block_table(block_table * table) {
    for (int i = 0; i < table->number_of_blocks; i++) {
        if (table->blocks[i] != 0) free(table->blocks[i]->wc_results);
        free(table->blocks[i]);
    }
    free(table->blocks);
    free(table);
}

void print_all_blocks(block_table * table) {
    for (int i = 0; i < table->number_of_blocks; i++) {
        if (table->blocks[i] != 0) {
            printf("%s", table->blocks[i]->wc_results);
        }
    }
}