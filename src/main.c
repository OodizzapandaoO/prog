#include <stdio.h>
#include <string.h>

#include "formatting_logic.h"

int main(int argc, char *argv[]){
	if(argc != 3 && argc != 2){
		fprintf(stderr, "Ну, как бы, надо 2 файла... \n");
		return 1;
	}
		
	FILE *pars_from_ptr = fopen(argv[1], "r");
	if(!pars_from_ptr){
		fprintf(stderr, "%s ", argv[1]);
		perror("не удалось открыть файл ");
		return 1;
	}
		
	FILE *pars_to_ptr = NULL;
	if( !strcmp(argv[2], "stdout") || (argc == 2)){
		pars_to_ptr = stdout;
	} else {
		pars_to_ptr = fopen(argv[2], "w");
		if(!pars_from_ptr){
			fprintf(stderr, "%s ", argv[2]);
			perror("не удалось создать файл ");
			return 1;
		}
	}
		
	formating(pars_from_ptr, pars_to_ptr);
		
	if(pars_to_ptr == stdout){
		fclose(pars_to_ptr);
	}
		fclose(pars_from_ptr);
        return 0;
}