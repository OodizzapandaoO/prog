#include "formatting_logic.h"

#include <ctype.h>
#include <string.h>

#define MAX_WORD_SIZE 128

static void out_block_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr);
static void block_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr);
static void pre_proc_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr);
static void arg_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr);


static void expr_table_fill(expr_func_ptr *table_ptr){
//	table_ptr[EXPR_NONE] = &block_expr;
	table_ptr[EXPR_PRE_PROC] = &pre_proc_expr;
	table_ptr[EXPR_BLOCK] = &block_expr;
	table_ptr[EXPR_OUT_BLOCK] = &out_block_expr;
	table_ptr[EXPR_ARG] = &arg_expr;
}

static void code_parser_stat_ptr_do(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	stat_ptr->table_ptr[stat_ptr->tail_ptr->current_expression](from_ptr, to_ptr, stat_ptr);
}

void skip_space(FILE *from_ptr){
	int ch = fgetc(from_ptr);
	while((isblank(ch)) && (ch != EOF)){
//		printf("SKIP ");
		ch = fgetc(from_ptr);
	}
	ungetc(ch, from_ptr);
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

static int word_start_detect(int c){
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

	while(word_start_detect(c)){
		word[len] = c;
		c = fgetc(from_ptr);
		len++;
	}
	word[len] = '\0';

	ungetc(c, from_ptr);
	return word;
}

static void new_line(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	int c = fgetc(from_ptr);
	if(c != '\n')
		ungetc(c, from_ptr);

	fprintf(to_ptr, "\n");
	if(c == '#'){
		return;
	}

	for(unsigned i = 0 ; i < stat_ptr->tab_count; i++){
		fprintf(to_ptr, "\t");
	}
	stat_ptr->line_char_count = 0;
}

static void print_word(char *word_ptr, FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	const int tab_len = 4;
	if((stat_ptr->tab_count * tab_len
		+ stat_ptr->line_char_count + tab_len + strlen(word_ptr)) > 79){

		new_line(from_ptr, to_ptr, stat_ptr);
		fprintf(to_ptr, "\t");
	}
	fprintf(to_ptr, "%s", word_ptr);
	stat_ptr->line_char_count += strlen(word_ptr);
	return;
}

static void set_if_absent(FILE *from_ptr, FILE *to_ptr, char c){
	int ch = fgetc(from_ptr);
	fprintf(to_ptr, "%c", c);
	if(ch != c){
		ungetc(ch, from_ptr);
	}
	return;
}

static int check_other_letter(char c){
	char letters[] = {' ', '*', '&', '=', '+', '-', ':', '?', '['
		, ']', '"', '.', '<', '>', '|', (char)39};

	for(int i = 0; i < sizeof(letters); i++){
		if(letters[i] == c){
			return 1;
		}
	}
	return 0;
}

static void out_block_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	int ch = fgetc(from_ptr);
	while(ch != EOF){
		if(ch == '#'){
			code_parser_stat_push(stat_ptr, EXPR_PRE_PROC);
			code_parser_stat_ptr_do(from_ptr, to_ptr, stat_ptr);
		} else if(word_start_detect(ch)){

			ungetc(ch, from_ptr);
			char *word = get_word(from_ptr);
			print_word(word, from_ptr, to_ptr, stat_ptr);

		}else if(ch == '('){
			code_parser_stat_push(stat_ptr, EXPR_ARG);
			code_parser_stat_ptr_do(from_ptr, to_ptr, stat_ptr);
		}else if(ch == '{'){
			code_parser_stat_push(stat_ptr, EXPR_BLOCK);
			code_parser_stat_ptr_do(from_ptr, to_ptr, stat_ptr);
		} else if(ch == ','){
			fprintf(to_ptr, ",");
			set_if_absent(from_ptr, to_ptr, ' ');
		} else if(ch == '\n'){
			new_line(from_ptr, to_ptr, stat_ptr);
		} else if(ch == ';'){
			fprintf(to_ptr, ";");
			set_if_absent(from_ptr, to_ptr, '\n');
		} else if(check_other_letter(ch)){
			fprintf(to_ptr, "%c", ch);
		}

		ch = fgetc(from_ptr);
	}
	return;
}

static void arg_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	fprintf(to_ptr, "(");
	int ch = fgetc(from_ptr);
	while((ch != ')') && (ch != EOF)){
		if(word_start_detect(ch)){
			ungetc(ch, from_ptr);
			char *word = get_word(from_ptr);
			print_word(word, from_ptr, to_ptr, stat_ptr);
		} else if(ch == ','){
			fprintf(to_ptr, ",");
			set_if_absent(from_ptr, to_ptr, ' ');
		} else if(ch == '\n'){
			new_line(from_ptr, to_ptr, stat_ptr);
		} else if(ch == ':'){
			fprintf(to_ptr, ":");
		} else if(check_other_letter(ch)){
			fprintf(to_ptr, "%c", ch);
		}
		ch = fgetc(from_ptr);
	}
	fprintf(to_ptr, ")");
	return;
}

static int check_block_end(FILE *from_ptr){
	int ch = fgetc(from_ptr);
	if(ch != '\n')
		ungetc(ch, from_ptr);
	skip_space(from_ptr);

	ch = fgetc(from_ptr);
	if(ch == '}')
		return 1;
	ungetc(ch, from_ptr);
	return 0;
}

static void block_expr(FILE *from_ptr, FILE *to_ptr, code_parser_stat_t *stat_ptr){
	fprintf(to_ptr, "{");
	stat_ptr->tab_count++;
	int ch = fgetc(from_ptr);
	while((ch != '}') && (ch != EOF)){
		if(ch == '#'){
			code_parser_stat_push(stat_ptr, EXPR_PRE_PROC);
			code_parser_stat_ptr_do(from_ptr, to_ptr, stat_ptr);
		} else if(word_start_detect(ch)){

			ungetc(ch, from_ptr);
			char *word = get_word(from_ptr);
			print_word(word, from_ptr, to_ptr, stat_ptr);

		}else if(ch == '('){
			code_parser_stat_push(stat_ptr, EXPR_ARG);
			code_parser_stat_ptr_do(from_ptr, to_ptr, stat_ptr);
		}else if(ch == '{'){
			code_parser_stat_push(stat_ptr, EXPR_BLOCK);
			code_parser_stat_ptr_do(from_ptr, to_ptr, stat_ptr);
		} else if(ch == ','){
			fprintf(to_ptr, ",");
			set_if_absent(from_ptr, to_ptr, ' ');
		} else if(ch == '\n'){
			if(check_block_end(from_ptr)){
				break;
			}
			new_line(from_ptr, to_ptr, stat_ptr);
		} else if(ch == ';'){
			fprintf(to_ptr, ";");
			if(check_block_end(from_ptr)){
				break;
			}
			new_line(from_ptr, to_ptr, stat_ptr);
		} else if(check_other_letter(ch)){
			fprintf(to_ptr, "%c", ch);
		}

		ch = fgetc(from_ptr);
	}

/*	ch = fgetc(from_ptr);
	if(ch != '\n'){
		ungetc(ch, from_ptr);
	}*/
	skip_space(from_ptr);

	stat_ptr->tab_count--;
	new_line(from_ptr, to_ptr, stat_ptr);

	fprintf(to_ptr, "}");
	ch = fgetc(from_ptr);
	if(ch != '}')
		new_line(from_ptr, to_ptr, stat_ptr);
	ungetc(ch, from_ptr);
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

	new_line(from_ptr, to_ptr, stat_ptr);
	code_parser_stat_pop(stat_ptr);
	return;
}
