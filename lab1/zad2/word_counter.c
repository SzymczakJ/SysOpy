#include "stdio.h"
#include "stdlib.h"
#include "blckmem.h"
#include "string.h"
#include "word_counter.h"
#include "ctype.h"
#include <sys/times.h>
#include <stdint.h>
#include <unistd.h>
clock_t clock_t_begin, clock_t_end;
struct tms times_start_buffer, times_end_buffer;

void start_timer(){
    clock_t_begin = times(&times_start_buffer);
}

void stop_timer(){
    clock_t_end = times(&times_end_buffer);
}

double calc_time(clock_t s, clock_t e) {
    return ((double) (e - s) / (double) sysconf(_SC_CLK_TCK));
}

void print_times(){
    printf("real %.6fs user %.6fs sys %.6fs\n",
           calc_time(clock_t_begin, clock_t_end),
           calc_time(times_start_buffer.tms_cutime, times_end_buffer.tms_cutime),
           calc_time(times_start_buffer.tms_cstime, times_end_buffer.tms_cstime));
}

void wc_test() {
    FILE * temp_file = tmpfile();
    char * small_files = "text_files/empty_file text_files/super_small_file text_files/small_file";
    printf("small files:\n");
    start_timer();
    count_words(temp_file, small_files, strlen(small_files));
    stop_timer();
    print_times();

    char * mediuml_files = "text_files/medium_file text_files/semi_medium_file";
    printf("medium files:\n");
    start_timer();
    count_words(temp_file, mediuml_files, strlen(mediuml_files));
    stop_timer();
    print_times();

    char * large_files = "text_files/semi_large_file text_files/large_file text_files/super_large_file";
    printf("large files:\n");
    start_timer();
    count_words(temp_file, large_files, strlen(large_files));
    stop_timer();
    print_times();

    printf("all files:\n");
    char * all_files = "text_files/semi_large_file text_files/large_file text_files/super_large_file text_files/medium_file text_files/semi_medium_file text_files/empty_file text_files/super_small_file text_files/small_file";
    start_timer();
    count_words(temp_file, all_files, strlen(all_files));
    stop_timer();
    print_times();

    fclose(temp_file);
}

void save_and_delete_test() {
    printf("save test:\n");
    block_table * table = NULL;
    char command[] = "create_table 10";
    char * scnd_comm = strtok(command, " ");
    table = parse_create_table(table, scnd_comm);
    char * save_part_commands[] = {
            "wc_files text_files/empty_file text_files/super_small_file text_files/small_file",
            "wc_files text_files/medium_file text_files/semi_medium_file",
            "wc_files text_files/semi_large_file text_files/large_file text_files/super_large_file",
            "wc_files text_files/semi_large_file text_files/large_file text_files/super_large_file text_files/medium_file text_files/semi_medium_file text_files/empty_file text_files/super_small_file text_files/small_file"};
    char * file_comments[] = {
            "small files:",
            "medium files:",
            "large files:",
            "all files:"
    };

    char parse_helper[500];
    for (int i = 0; i < 4; i++) {
        strcpy(parse_helper, save_part_commands[i]);
        char * scnd_comm1 = strtok(parse_helper, " ");
        printf("%s:\n", file_comments[i]);
        start_timer();
        parse_wc_files(table, scnd_comm1);
        stop_timer();
        print_times();
    }

    printf("delete test:\n");
    char * block_indexes[] = {"c 0", "d 1", "d 2", "d 3"};
    for (int i = 0; i < 4; i++) {
        strcpy(parse_helper, block_indexes[i]);
        char * scnd_comm1 = strtok(parse_helper, " ");
        printf("%s:\n", file_comments[i]);
        start_timer();
        parse_remove_block(table, scnd_comm1);
        stop_timer();
        print_times();
    }
}

void few_saves_deletes_test() {
    printf("save->delete x10 test:\n");
    block_table * table = NULL;
    char command[] = "create_table 10";
    char * scnd_comm = strtok(command, " ");
    table = parse_create_table(table, scnd_comm);
    char * save_part_commands[] = {
            "wc_files text_files/empty_file text_files/super_small_file text_files/small_file",
            "wc_files text_files/medium_file text_files/semi_medium_file",
            "wc_files text_files/semi_large_file text_files/large_file text_files/super_large_file",
            "wc_files text_files/semi_large_file text_files/large_file text_files/super_large_file text_files/medium_file text_files/semi_medium_file text_files/empty_file text_files/super_small_file text_files/small_file"};
    char * file_comments[] = {
            "small files:",
            "medium files:",
            "large files:",
            "all files:"
    };

    char parse_helper[500];
    start_timer();
    for (int i = 0; i < 10; i++) {
        for (int i = 0; i < 4; i++) {
            strcpy(parse_helper, save_part_commands[i]);
            char * scnd_comm1 = strtok(parse_helper, " ");
            parse_wc_files(table, scnd_comm1);
        }

        char * block_indexes[] = {"c 0", "d 1", "d 2", "d 3"};
        for (int i = 0; i < 4; i++) {
            strcpy(parse_helper, block_indexes[i]);
            char * scnd_comm1 = strtok(parse_helper, " ");
            parse_remove_block(table, scnd_comm1);
        }
    }
    stop_timer();
    print_times();
}

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
#ifndef TEST
    sprintf(index_info, "Allocated block on index: %d", block_index);
    puts(index_info);
#endif
    return;
}




void parse_print_blocks(block_table * table) {
    print_all_blocks(table);
}


int main(int argc, char** argv) {
#ifdef TEST
    wc_test();
    save_and_delete_test();
    few_saves_deletes_test();
    return 0;
#endif
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
        else puts("invalid command");
    }
    if (table != NULL) delete_block_table(table);
}