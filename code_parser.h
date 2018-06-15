#ifndef CODE_PARSER_H
#define CODE_PARSER_H

typedef enum{
	NONE,	
	PRE_PROCESSOR
}expr_t;

struct expr_node_s;
typedef struct expr_node_s expr_node_t;

struct expr_node_s{
	expr_node_t *next_node;
	expr_t current_expression;
};

typedef struct{
	unsigned int line_char_count;
	unsigned int tab_count;
	expr_node_t *head;
	expr_node_t *tail;
} code_parser_stat_t;

void code_parser_stat_init(code_parser_stat_t *parser_stat);
void code_parser_stat_free(code_parser_stat_t *parser_stat);
void code_parser_stat_push(code_parser_stat_t *parser_stat, expr_t new_expression);
expr_t code_parser_stat_pop(code_parser_stat_t *parser_stat);

#endif


