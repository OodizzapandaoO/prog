#include "code_parser.h"
#include <stdlib.h>

void code_parser_stat_init(code_parser_stat_t *parser_stat_ptr, expr_func_ptr *table_ptr){
	parser_stat_ptr->table_ptr = table_ptr;
	parser_stat_ptr->line_char_count = 0;
	parser_stat_ptr->tab_count = 0;
	parser_stat_ptr->head_ptr = NULL;
	parser_stat_ptr->tail_ptr = NULL;
}

void code_parser_stat_free(code_parser_stat_t *parser_stat_ptr){
	expr_node_t *current_node_ptr = parser_stat_ptr->head_ptr;
	expr_node_t *next_node_ptr = NULL;
	while(current_node_ptr != NULL){
		next_node_ptr = current_node_ptr->next_node_ptr;
		free(current_node_ptr);
		current_node_ptr = next_node_ptr;
	}
	return;
}

void code_parser_stat_push(code_parser_stat_t *parser_stat_ptr, expr_t new_expression){
	
	expr_node_t *new_node_ptr = malloc(sizeof(expr_node_t));
	
	new_node_ptr->next_node_ptr = NULL;
	new_node_ptr->current_expression = new_expression;
		
	if(parser_stat_ptr->head_ptr == NULL){
		parser_stat_ptr->head_ptr = new_node_ptr;
		parser_stat_ptr->tail_ptr = new_node_ptr;
	} else {
		parser_stat_ptr->tail_ptr->next_node_ptr = new_node_ptr;
		parser_stat_ptr->tail_ptr = new_node_ptr;
	}
	return;
}

expr_t code_parser_stat_pop(code_parser_stat_t *parser_stat_ptr){
	expr_node_t *previous_node_ptr = NULL;
	expr_node_t *current_node_ptr = parser_stat_ptr->head_ptr;
	while(current_node_ptr == parser_stat_ptr->tail_ptr){
		previous_node_ptr = current_node_ptr;
		current_node_ptr = current_node_ptr->next_node_ptr;
	}
	if(previous_node_ptr == NULL){
		parser_stat_ptr->head_ptr = NULL;
	}
	parser_stat_ptr->tail_ptr = previous_node_ptr;
	if(previous_node_ptr != NULL){
		previous_node_ptr->next_node_ptr = NULL;	
	}
	expr_t result = EXPR_NONE;
	if(current_node_ptr != NULL){
		result = current_node_ptr->current_expression;
		free(current_node_ptr);
	}
	return result;
}
