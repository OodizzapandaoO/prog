#include "formatting_logic.h"

#include <ctype.h>

#define MAX_WORD_SIZE 128

typedef void (*expr_func)(FILE *from, FILE *to, code_parser_stat_t *stat);

static void block_expr(FILE *from, FILE *to, code_parser_stat_t *stat);
static void pre_proc_expr(FILE *from, FILE *to, code_parser_stat_t *stat);

static void expr_table_fill(expr_func *table){
	table[EXPR_NONE] = &block_expr;
	table[EXPR_PRE_PROC] = &pre_proc_expr;
	table[EXPR_BLOCK] = &block_expr;
}

void formating(FILE *from, FILE *to){
	
	size_t expr_size = EXPR_LAST + 1;
	expr_func expr_table[expr_size];
	expr_table_fill(expr_table);
	
/*	fgetc(from);
	char *word_test = get_word(from);
	fprintf(to, "%s\n", word_test);*/
	
}

static int word_condition(int c){
	if(c == EOF)
		return 0;
	
	if(isalnum(c) || (c == '_'))
		return 1;
	else
		return 0;
}

char *get_word(FILE *from){
	static char word[MAX_WORD_SIZE];
	
	int c = fgetc(from);
	int len = 0;
	
	while(word_condition(c)){
		word[len] = c;
		c = fgetc(from);
		len++;
	}
	word[len] = '\0';
	
	ungetc(c, from);
	return word;
}

static void block_expr(FILE *from, FILE *to, code_parser_stat_t *stat){
	
}

static void pre_proc_expr(FILE *from, FILE *to, code_parser_stat_t *stat){
	
}