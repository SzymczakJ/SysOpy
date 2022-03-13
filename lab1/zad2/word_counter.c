#include "stdio.h"
#include "stdlib.h"
#include "blckmem.h"
#include "string.h"
#include "word_counter.h"
#include "ctype.h"

int is_integer(char * string) {
    while (*string != 0x0) {
        if (!isdigit(*(string++))) return 0;
    }
    return 1;
}

block_table * parse_create_table(block_table * table, char * token) {
    token = strtok(NULL, " ");
    if (token == NULL || !is_integer(token)) {
        puts("Invalid parameter.");
    }
    else {
        if (table != NULL) {
            delete_block_table(table);
        }
        table = create_block_table(atoi(token));
    }
    return table;
}

void parse_remove_block(block_table * table, char * token) {
    token = strtok(NULL, " ");
    if (table == NULL) puts("Table hasn't been created yet.");
    if (token == NULL) puts("Invalid parameter.");
    else remove_blocks(table, atoi(token));
}

void parse_wc_files(block_table * table, char * token) {
    token = strtok(NULL, "\n");
    puts(token);
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
    fclose(temp_file);
    return;
}

void parse_print_blocks(block_table * table) {
    print_all_blocks(table);
}

int main() {
    block_table * table = NULL;
    char * line = NULL;
    size_t buffer_size;
    while (1) {
        printf(">");
        size_t num_read = getline(&line, &buffer_size, stdin);
        line[num_read - 1] = 0x0;
        char * command = strtok(line, " ");
        if (strcmp(command, "create_table") == 0) table = parse_create_table(table, command);
        else if (strcmp(command, "remove_block") == 0) parse_remove_block(table, command);
        else if (strcmp(command, "wc_files") == 0) parse_wc_files(table, command);
        else if (strcmp(command, "print_all_blocks") == 0) parse_print_blocks(table);
        else if (strcmp(command, "exit") == 0) break;
        else puts("Invalic command");
    }
    if (table != NULL) delete_block_table(table);
}