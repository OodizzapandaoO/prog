#include "code_parser.h"
#include <stdlib.h>

void code_parser_stat_init(code_parser_stat_t *parser_stat){
	parser_stat->line_char_count = 0;
	parser_stat->tab_count = 0;
	parser_stat->head = NULL;
	parser_stat->tail = NULL;
}

void code_parser_stat_free(code_parser_stat_t *parser_stat){
	expr_node_t *current_node = parser_stat->head;
	expr_node_t *next_node = NULL;
	while(current_node != NULL){
		next_node = current_node->next_node;
		free(current_node);
		current_node = next_node;
	}
	return;
}

void code_parser_stat_push(code_parser_stat_t *parser_stat, expr_t new_expression){
	expr_node_t *new_node = malloc(sizeof(expr_node_t));
	new_node->next_node = NULL;
	new_node->current_expression = new_expression;
	parser_stat->tail->next_node = new_node;
	parser_stat->tail = new_node;
	if(parser_stat->head == NULL){
		parser_stat->head = new_node;
	}
}

expr_t code_parser_stat_pop(code_parser_stat_t *parser_stat){
	expr_node_t *previous_node = NULL;
	expr_node_t *current_node = parser_stat->head;
	while(current_node == parser_stat->tail){
		previous_node = current_node;
		current_node = current_node->next_node;
	}
	if(previous_node == NULL){
		parser_stat->head = NULL;
	}
	parser_stat->tail = previous_node;
	if(previous_node != NULL){
		previous_node->next_node = NULL;	
	}
	expr_t result = EXPR_NONE;
	if(current_node != NULL){
		result = current_node->current_expression;
		free(current_node);
	}
	return result;
}
