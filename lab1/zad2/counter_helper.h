//
// Created by kubson on 13.03.2022.
//

#ifndef LAB1_COUNTER_HELPER_H
#define LAB1_COUNTER_HELPER_H

block_table * parse_create_table(block_table * table, char * token);
int is_integer(char * string);
void parse_remove_block(block_table * table, char * token);
void parse_wc_files(block_table * table, char * token);
void parse_print_blocks(block_table * table);
#endif //LAB1_COUNTER_HELPER_H
