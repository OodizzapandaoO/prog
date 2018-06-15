#include "formatting_logic.h"

#include <ctype.h>

#define MAX_WORD_SIZE 128

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