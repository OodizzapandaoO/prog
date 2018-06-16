#ifndef FORMATTING_LOGIC_H
#define FORMATTING_LOGIC_H

#include <stdio.h>

#include "code_parser.h"

void formating(FILE *from_ptr, FILE *to_ptr);

char *get_word(FILE *from_ptr);

#endif