#include "blckmem.h"
#include "stdio.h"
#include "stdlib.h"

int is_integer(char * string) {
    while (*string != 0x0) {
        if (!isdigit(*(string++))) return 0;
    }
    return 1;
}

block_table * parse_create_table(block_table * table, char * token) {
    token = strtok(NULL, " ");
    if (token == NULL || !is_integer(token)) {
        puts("Invalid parameter.")
    }
    else {
        if (table != NULL) {
            delete_block_table(table);
        }
        table = create_block_table();
    }
    return table;
}

void parse_remove_block(block_table * table, char * token) {
    token = strtok(NULL, " ");
    if (table == NULL) puts("Table hasn't been created yet.");
    if (token == NULL) puts("Invalid parameter.")
    else remove_blocks(table, block_index);
}

void parse_wc_files(block_table * table, char * token) {
    token = strtok(NULL, "\n");
    if (token == NULL) {
        puts("Invalid parameter.");
        return;
    }
    FILE * temp_file = tmpfile();
    count_words(temp_file, token, strlen(token));
    int block_index = write_from_file_to_block(temp_file, table);
    char index_info[50];
    sprintf(index_info, "Allocated block on index: %d", block_index);
    puts(index_info);
    return;
}

void parse_print_blocks(block_table * table) {
    print_all_blocks(table);
}