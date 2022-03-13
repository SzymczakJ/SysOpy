#include "stdio.h"
#include "stdlib.h"
#include "blckmem.h"
#include "counter_helper.h"

int main() {
    block_table * table = NULL;
    char * line = NULL;
    size_t buffer_size;
    while (true) {
        printf(">");
        size_t num_read = getline(&line, &buffer_size, stdin);
        line[num_read - 1] = 0x0;
        char * command = strtok(line, " ");
        if (strcmp(command, "create_table") == 0) table = parse_create_table(table, command);
        else if (strcmp(command, "remove_block") == 0)
    }
}