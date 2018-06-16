#include "formatting_logic.h"

#include <ctype.h>
#include <string.h>

#define MAX_WORD_SIZE 128

static void out_block_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr);
static void block_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr);
static void pre_proc_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr);

static void expr_table_fill(expr_func_ptr *table_ptr){
//	table_ptr[EXPR_NONE] = &block_expr;
	table_ptr[EXPR_PRE_PROC] = &pre_proc_expr;
	table_ptr[EXPR_BLOCK] = &block_expr;
	table_ptr[EXPR_OUT_BLOCK] = &out_block_expr;
}

static void code_parser_stat_ptr_do(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	stat_ptr->table_ptr[stat_ptr->tail_ptr->current_expression](from_ptr, to_ptr, stat_ptr);
}

void formating(FILE *from_ptr, FILE *to_ptr){

	size_t expr_size = EXPR_LAST + 1;
	expr_func_ptr expr_table[expr_size];
	expr_table_fill(expr_table);
	
	code_parser_stat_t stat_ptr;
	memset(&stat_ptr, 0, sizeof(code_parser_stat_t));

	code_parser_stat_init(&stat_ptr, expr_table);
	
	code_parser_stat_push(&stat_ptr, EXPR_OUT_BLOCK);
	code_parser_stat_ptr_do(from_ptr, to_ptr, &stat_ptr);
	
	code_parser_stat_free(&stat_ptr);
}

static int word_condition(int c){
	if(c == EOF)
		return 0;
	
	if(isalnum(c) || (c == '_'))
		return 1;
	else
		return 0;
}

char *get_word(FILE *from_ptr){
	static char word[MAX_WORD_SIZE];
	
	int c = fgetc(from_ptr);
	int len = 0;
	
	while(word_condition(c)){
		word[len] = c;
		c = fgetc(from_ptr);
		len++;
	}
	word[len] = '\0';
	
	ungetc(c, from_ptr);
	return word;
}

static void out_block_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	int ch = fgetc(from_ptr);
	while(ch != EOF){
		if(ch == '#'){
			code_parser_stat_push(stat_ptr, EXPR_PRE_PROC);
			code_parser_stat_ptr_do(from_ptr, to_ptr, stat_ptr);
		} else if(word_condition(ch)){
			
		} else if(ch == '\n'){
			fprintf(to_ptr, "\n");
			stat_ptr->line_char_count = 0;
		}
		
		ch = fgetc(from_ptr);
	}
	return;
}

static void block_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	int ch = fgetc(from_ptr);
	while((ch != '}') && (ch != EOF)){
		
	}
	
	ch = fgetc(from_ptr);
	if(ch != '\n')
		ungetc(ch, from_ptr);
	
	fprintf(to_ptr, "}\n");
	stat_ptr->line_char_count = 0;
}

static void pre_proc_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	if(stat_ptr->line_char_count != 0){
		stat_ptr->line_char_count = 0;
		fprintf(to_ptr, "\n");
	}
	
	fprintf(to_ptr, "#");
	stat_ptr->line_char_count++;
	
	int ch = fgetc(from_ptr);
	while((ch != '\n') && (ch != EOF)){
		
		stat_ptr->line_char_count++;
		fprintf(to_ptr, "%c", ch);
		
		if(stat_ptr->line_char_count == 79){
			fprintf(to_ptr, "\\\n");
			stat_ptr->line_char_count = 0;
		}
		
		int prev_ch = ch;
		ch = fgetc(from_ptr);		
		if((ch == '\n') && (prev_ch == '\\')){
			fprintf(to_ptr, "%c", ch);
			ch = fgetc(from_ptr);
		}
	}
	fprintf(to_ptr, "\n");
	stat_ptr->line_char_count = 0;
	
	code_parser_stat_pop(stat_ptr);
	return;
}