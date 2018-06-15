#include "code_parser.h"
#include <stdio.h>

int main(int argc, char *argv[]){
        if(argc != 3){
                fprintf(stderr, "Ну, как бы, надо 2 файла... \n");
                return 1;
        }
        FILE *pars_from = fopen(argv[1], "r");
        if(!pars_from){
                fprintf(stderr, "%s ", argv[1]);
                perror("не удолось открыть файл ");
                return 1;
        }
        FILE *pars_to = fopen(argv[2], "w");
        if(!pars_from){
                fprintf(stderr, "%s ", argv[2]);
                perror("не удолось создать файл ");
                return 1;
        }
        return 0;
}
