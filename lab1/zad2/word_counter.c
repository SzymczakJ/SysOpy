#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include <sys/times.h>
#include <stdint.h>
#include <unistd.h>

#ifdef LIB_DYNAMIC
#include <dlfcn.h>
#else
#include "blckmem.h"
#endif

#ifdef LIB_DYNAMIC
        typedef struct {
        size_t result_size;
        char * wc_results;
        }memory_block;

        typedef struct {
        size_t number_of_blocks;
        }block_table;
#endif

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


int is_integer(char * string) {
    while (*string != 0x0) {
        if (!isdigit(*(string++))) return 0;
    }
    return 1;
}

block_table * parse_create_table(block_table * table, char * token, void * handle) {
#ifdef LIB_DYNAMIC
    block_table* (*create_block_table)(int) = (block_table *(*)(int)) dlsym(handle, "create_block_table");
    void (*delete_block_table)(block_table*) = (void(*)(block_table*)) dlsym(handle, "delete_block_table");
    if (dlerror()) {
            fprintf(stderr, "Cannot load functions from library.so\n");
            exit(1);
        }
#endif
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

void parse_remove_block(block_table * table, char * token, void * handle) {
#ifdef LIB_DYNAMIC
    void (*remove_blocks)(block_table*, int) = (void(*)(block_table*, int)) dlsym(handle, "remove_blocks");
    if (dlerror()) {
            fprintf(stderr, "Cannot load functions from library.so\n");
            exit(1);
        }
#endif
    token = strtok(NULL, " ");
    if (table == NULL) puts("Table hasn't been created yet.");
    if (token == NULL) puts("Invalid parameter.");
    else remove_blocks(table, atoi(token));
}

void parse_wc_files(block_table * table, char * token, void * handle) {
#ifdef LIB_DYNAMIC
    void (*count_words)(FILE*, char*, int) = (void (*) (FILE*, char*, int)) dlsym(handle, "count_words");
    int (*write_from_file_to_block)(FILE*, block_table*) = (int(*)(FILE*, block_table*)) dlsym(handle, "write_from_file_to_block");
    if (dlerror()) {
            fprintf(stderr, "Cannot load functions from library.so\n");
            exit(1);
        }
#endif
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




void parse_print_blocks(block_table * table, void * handle) {
#ifdef LIB_DYNAMIC
    void (*print_all_blocks)(block_table*) = (void(*)(block_table*)) dlsym(handle, "print_all_blocks");
    if (dlerror()) {
            fprintf(stderr, "Cannot load functions from library.so\n");
            exit(1);
        }
#endif
    print_all_blocks(table);
}

void wc_test(void * handle) {
#ifdef LIB_DYNAMIC
    void (*count_words)(FILE*, char*, int) = (void (*) (FILE*, char*, int)) dlsym(handle, "count_words");
    if (dlerror()) {
            fprintf(stderr, "Cannot load functions from library.so\n");
            exit(1);
        }
#endif
    FILE * temp_file = tmpfile();
    char * small_files = "../zad2/text_files/empty_file ../zad2/text_files/super_small_file ../zad2/text_files/small_file";
    printf("small files:\n");
    start_timer();
    count_words(temp_file, small_files, strlen(small_files));
    stop_timer();
    print_times();

    char * mediuml_files = "../zad2/text_files/medium_file ../zad2/text_files/semi_medium_file";
    printf("medium files:\n");
    start_timer();
    count_words(temp_file, mediuml_files, strlen(mediuml_files));
    stop_timer();
    print_times();

    char * large_files = "../zad2/text_files/semi_large_file ../zad2/text_files/large_file ../zad2/text_files/super_large_file";
    printf("large files:\n");
    start_timer();
    count_words(temp_file, large_files, strlen(large_files));
    stop_timer();
    print_times();

    printf("all files:\n");
    char * all_files = "../zad2/text_files/semi_large_file ../zad2/text_files/large_file ../zad2/text_files/super_large_file ../zad2/text_files/medium_file ../zad2/text_files/semi_medium_file ../zad2/text_files/empty_file ../zad2/text_files/super_small_file ../zad2/text_files/small_file";
    start_timer();
    count_words(temp_file, all_files, strlen(all_files));
    stop_timer();
    print_times();

    fclose(temp_file);
}

void save_and_delete_test(void * handle) {
#ifdef LIB_DYNAMIC
    void (*remove_blocks)(block_table*, int) = (void(*)(block_table*, int)) dlsym(handle, "remove_blocks");
    if (dlerror()) {
            fprintf(stderr, "Cannot load functions from library.so\n");
            exit(1);
        }
#endif
    printf("save test:\n");
    block_table * table = NULL;
    char command[] = "create_table 10";
    char * scnd_comm = strtok(command, " ");
    table = parse_create_table(table, scnd_comm, handle);
    char * save_part_commands[] = {
            "wc_files ../zad2/text_files/empty_file ../zad2/text_files/super_small_file ../zad2/text_files/small_file",
            "wc_files ../zad2/text_files/medium_file ../zad2/text_files/semi_medium_file",
            "wc_files ../zad2/text_files/semi_large_file ../zad2/text_files/large_file ../zad2/text_files/super_large_file",
            "wc_files ../zad2/text_files/semi_large_file ../zad2/text_files/large_file ../zad2/text_files/super_large_file ../zad2/text_files/medium_file ../zad2/text_files/semi_medium_file ../zad2/text_files/empty_file ../zad2/text_files/super_small_file ../zad2/text_files/small_file"};
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
        parse_wc_files(table, scnd_comm1, handle);
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
        parse_remove_block(table, scnd_comm1, handle);
        stop_timer();
        print_times();
    }
}

void few_saves_deletes_test(void * handle) {
#ifdef LIB_DYNAMIC
    void (*remove_blocks)(block_table*, int) = (void(*)(block_table*, int)) dlsym(handle, "remove_blocks");
    if (dlerror()) {
            fprintf(stderr, "Cannot load functions from library.so\n");
            exit(1);
        }
#endif
    printf("save->delete x10 test:\n");
    block_table * table = NULL;
    char command[] = "create_table 10";
    char * scnd_comm = strtok(command, " ");
    table = parse_create_table(table, scnd_comm, handle);
    char * save_part_commands[] = {
            "wc_files ../zad2/text_files/empty_file ../zad2/text_files/super_small_file ../zad2/text_files/small_file",
            "wc_files ../zad2/text_files/medium_file ../zad2/text_files/semi_medium_file",
            "wc_files ../zad2/text_files/semi_large_file ../zad2/text_files/large_file ../zad2/text_files/super_large_file",
            "wc_files ../zad2/text_files/semi_large_file ../zad2/text_files/large_file ../zad2/text_files/super_large_file ../zad2/text_files/medium_file ../zad2/text_files/semi_medium_file ../zad2/text_files/empty_file ../zad2/text_files/super_small_file ../zad2/text_files/small_file"};

    char parse_helper[500];
    start_timer();
    for (int i = 0; i < 10; i++) {
        for (int i = 0; i < 4; i++) {
            strcpy(parse_helper, save_part_commands[i]);
            char * scnd_comm1 = strtok(parse_helper, " ");
            parse_wc_files(table, scnd_comm1, handle);
        }

        char * block_indexes[] = {"c 0", "d 1", "d 2", "d 3"};
        for (int i = 0; i < 4; i++) {
            strcpy(parse_helper, block_indexes[i]);
            char * scnd_comm1 = strtok(parse_helper, " ");
            parse_remove_block(table, scnd_comm1, handle);
        }
    }
    stop_timer();
    print_times();
}


int main(int argc, char** argv) {
    void *handle = NULL;
#ifdef LIB_DYNAMIC
    handle = dlopen((const char*) "../zad2/blckmem.so", RTLD_LAZY);
    if (!handle) {
            fprintf(stderr, "Cannot load library.so\n");
            exit(4);
        }
    void (*delete_block_table)(block_table*) = (void(*)(block_table*)) dlsym(handle, "delete_block_table");
    if (dlerror()) {
            fprintf(stderr, "Cannot load functions from library.so\n");
            exit(1);
        }
#endif
#ifdef TEST
    wc_test(handle);
    save_and_delete_test(handle);
    few_saves_deletes_test(handle);
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
        if (strcmp(command, "create_table") == 0) table = parse_create_table(table, command, handle);
        else if (strcmp(command, "remove_block") == 0) parse_remove_block(table, command, handle);
        else if (strcmp(command, "wc_files") == 0) parse_wc_files(table, command, handle);
        else if (strcmp(command, "print_all_blocks") == 0) parse_print_blocks(table, handle);
        else if (strcmp(command, "exit") == 0) break;
        else puts("invalid command");
    }
    if (table != NULL) delete_block_table(table);
#ifdef LIB_DYNAMIC
    dlclose(handle);
#endif
}