#include "blckmem.h"
#include "stdio.h"
#include "stdlib.h"

bool is_integer(char * string) {
    while (*string != 0x0) {
        if (!isdigit(*(string++))) return false;
    }
    return true;
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

void parse_remove_block(block_table * table, int block_index) {
    
}