#include "formatting_logic.h"

#include <ctype.h>
#include <string.h>

#define MAX_WORD_SIZE 128

static void out_block_expr(FILE *from, FILE *to, code_parser_stat_t *stat);
static void block_expr(FILE *from, FILE *to, code_parser_stat_t *stat);
static void pre_proc_expr(FILE *from, FILE *to, code_parser_stat_t *stat);

static void expr_table_fill(expr_func *table){
//	table[EXPR_NONE] = &block_expr;
	table[EXPR_PRE_PROC] = &pre_proc_expr;
	table[EXPR_BLOCK] = &block_expr;
	table[EXPR_OUT_BLOCK] = &out_block_expr;
}

static void code_parser_stat_do(FILE *from, FILE *to, code_parser_stat_t *stat){
	stat->table[stat->tail->current_expression](from, to, stat);
}

void formating(FILE *from, FILE *to){

	size_t expr_size = EXPR_LAST + 1;
	expr_func expr_table[expr_size];
	expr_table_fill(expr_table);
	
	code_parser_stat_t stat;
	memset(&stat, 0, sizeof(code_parser_stat_t));

	code_parser_stat_init(&stat, expr_table);
	
	code_parser_stat_push(&stat, EXPR_OUT_BLOCK);
	code_parser_stat_do(from, to, &stat);
	
	code_parser_stat_free(&stat);
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

static void out_block_expr(FILE *from, FILE *to, code_parser_stat_t *stat){
	int ch = fgetc(from);
	while(ch != EOF){
		if(ch == '#'){
			code_parser_stat_push(stat, EXPR_PRE_PROC);
			code_parser_stat_do(from, to, stat);
		} else if(word_condition(ch)){
			
		}
			
		ch = fgetc(from);
	}
	return;
}

static void block_expr(FILE *from, FILE *to, code_parser_stat_t *stat){
	int ch = fgetc(from);
	while((ch != '}') || (ch != EOF)){
		
	}
	fprintf(to, "}\n");
}

static void pre_proc_expr(FILE *from, FILE *to, code_parser_stat_t *stat){
	if(stat->line_char_count != 0){
		stat->line_char_count = 0;
		fprintf(to, "\n");
	}
	
	fprintf(to, "#");
	
	int ch = fgetc(from);
	while((ch != '\n') && (ch != EOF)){
		
		stat->line_char_count++;
		fprintf(to, "%c", ch);
		
		if(stat->line_char_count == 79){
			fprintf(to, "\\\n");
			stat->line_char_count = 0;
		}
		
		int prev_ch = ch;
		ch = fgetc(from);		
		if((ch == '\n') && (prev_ch == '\\')){
			fprintf(to, "%c", ch);
			ch = fgetc(from);
		}
	}
	fprintf(to, "\n");
	stat->line_char_count = 0;
	
	code_parser_stat_pop(stat);
	return;
}