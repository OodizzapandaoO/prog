#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include <stdio.h>

typedef enum{
	EXPR_FIRST = 0,
	
	EXPR_NONE = 0,	
	EXPR_PRE_PROC = 1,
	EXPR_BLOCK = 2,
	EXPR_OUT_BLOCK = 3,
	
	EXPR_LAST = 3,
}expr_t;

struct expr_node_s;
typedef struct expr_node_s expr_node_t;

struct expr_node_s{
	expr_node_t *next_node;
	expr_t current_expression;
};

struct code_parser_stat_s;
typedef struct code_parser_stat_s code_parser_stat_t;

typedef void (*expr_func)(FILE *from, FILE *to, code_parser_stat_t *stat);

struct code_parser_stat_s{
	expr_func *table;
	unsigned int line_char_count;
	unsigned int tab_count;
	expr_node_t *head;
	expr_node_t *tail;
};

void code_parser_stat_init(code_parser_stat_t *parser_stat, expr_func *table);
void code_parser_stat_free(code_parser_stat_t *parser_stat);
void code_parser_stat_push(code_parser_stat_t *parser_stat, expr_t new_expression);
expr_t code_parser_stat_pop(code_parser_stat_t *parser_stat);

#endif


