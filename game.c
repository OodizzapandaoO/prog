#include "game.h"
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#define ENG_WORDS

#define ESC_KEYCODE 0x09

static void set_title(char *title_ptr, Display *display, Window window){
	XTextProperty windowname;							
	XStringListToTextProperty (&title_ptr, 1, &windowname);

	XSetWMProperties ( display, window, &windowname,
  		NULL, NULL, NULL, NULL, NULL,
  		NULL );
}

int window_init(x_window_param_t *param, char *title,				
		int pos_x, int pos_y, int Width, int Height){
			
	param->display = XOpenDisplay(NULL);
	if(param->display == NULL){
		perror("Open display failed");
        return 1;
	}
	
	param->screen_number = DefaultScreen(param->display);
	
	param->window =  XCreateSimpleWindow(
			param->display, RootWindow(param->display, param->screen_number), sudo
            		BlackPixel(param->display, param->screen_number),
			WhitePixel(param->display, param->screen_number));
	
	set_title(title, param->display, param->window);
	
	XSelectInput(param->display, param->window, KeyPressMask | KeyReleaseMask );
	XMapWindow(param->display, param->window);
	
	param->gc = XCreateGC ( param->display, param->window, 0 , NULL );
	
	return 0;
}

int str_vec_init(string_vec_t *vec, unsigned capacity){
	vec->content = malloc(capacity * sizeof(char **));
	
	if(!vec->content){
		perror("Vector init allocation failed");
		return 1;
	}
	
	vec->capacity = capacity;
	vec->size = 0;
	
	return 0;
}

int str_vec_push(string_vec_t *vec, char *value){
	if(value[0] == '\0')
		return 0;
	
	if(vec->size >= vec->capacity){	
		
		unsigned new_capacity = vec->capacity * 2;
		char ** new_content = realloc(vec->content, new_capacity * sizeof(char **));
		if(!new_content){
			perror("Vector push reallocation failed");
			return 1;
		}
		
		vec->capacity = new_capacity;
		vec->content = new_content;
	} 
	
	int value_len = strlen(value) + 1;
	vec->content[vec->size] = malloc(value_len * sizeof(char));
	if(!vec->content[vec->size]){
		perror("Vector push allocation failed");
		return 1;
	}
	
	strncpy(vec->content[vec->size], value, value_len);
	++vec->size;

	return 0;
}

void str_vec_free(string_vec_t *vec){
	if(vec->content == NULL)
		return;
	
	for(int n = 0; n < vec->size; ++n){
		 free(vec->content[n]);
	}
	free(vec->content);
}

char *str_vec_random(string_vec_t *vec){
	srand(time(NULL));
	int random_variable = rand();
	return vec->content[random_variable % vec->size];
}

static char *merge_str(char *first, char *second){
	int size = strlen(first);
	size += strlen(second);	
	++size;
	
	char *result = malloc(size * sizeof(char));
	strncpy(result, first, strlen(first));
	strncpy(result + strlen(first), second, strlen(second));
	result[size] = '\0';
	return result;
}

int	str_vec_load_from_file(string_vec_t *vec, char *filedir, char *filename){
	str_vec_free(vec);
	str_vec_init(vec, 10);
	
	char *fullpath = merge_str(filedir, filename);
	
	FILE* words_file = fopen(fullpath, "r");
	if(!words_file) {
		perror("Open words file failed");
        return 1;
    }
	
	free(fullpath);	
	
	char current_word[100];
	int err = 0;
	
	while(!feof(words_file)){
		fscanf(words_file, "%s", current_word);
		
		err = str_vec_push(vec, current_word);
		if(err != 0){
			fprintf(stderr,"Vector push failed.\n");
			return 1;
		}
		
		current_word[0] = '\0';
	}
	
	return 0;
}

int load_pixmap(x_window_param_t *window, pixmap_attr_t *pixmap,
		char *filedir, char *filename){
			
	char *fullpath = merge_str(filedir, filename);
	
	int rc = XReadBitmapFile(window->display, window->window,
             fullpath,
             &pixmap->bitmap_width, &pixmap->bitmap_height,
             &pixmap->bitmap,
             &pixmap->x, &pixmap->y);
	
	if(rc != BitmapSuccess){
		fprintf(stderr,"Read bitmap failed: %s\n", fullpath);
		return 1;
	}
	
	free(fullpath);
	return 0;
}

int game_res_init(x_window_param_t *window, game_res_t *res, char *path){
	memset(&res->words, 0, sizeof(string_vec_t));

#ifdef ENG_WORDS
	int err = str_vec_load_from_file(&res->words, path, "words_eng.txt");
#else
	int err = str_vec_load_from_file(&res->words, path, "words.txt");
#endif

	if(err != 0)
		goto error_handler_1;
	
	int count = 0;
	
	for( ; count < 7; count++){
		
		char image_name[11];
		sprintf(image_name, "pos_%i.xbm", count);

		err = load_pixmap(window, &res->step_to_death[count], path, image_name);\
			if(err != 0)
				goto error_handler_2;
	}
	
	return 0;
	
error_handler_2:
	do{
		count--;
		XFreePixmap(window->display, res->step_to_death[count].bitmap);
	}while(count > 0);

	str_vec_free(&res->words);
	
error_handler_1:
	fprintf(stderr,"Game resources loading failed.\n");
	return 1;
}

void game_res_free(x_window_param_t *window, game_res_t *res){
	str_vec_free(&res->words);
	
	for(int i = 0; i < 6 ; i++)
		XFreePixmap(window->display, res->step_to_death[i].bitmap);
}

int game_init(game_stat_t *game, game_res_t *game_res){
	game->words_base = &game_res->words;
	game->word_progress = NULL;
	if(game_reset(game)){
		fprintf(stderr,"Game reset failed.\n");
		return 1;
	}
	return 0;
}

void game_letter_push(game_stat_t *game, char *letter){
	int hitting = 0;
	
	for(int i = 0; i < strlen(game->current_word); ++i){
		if(		(letter[0] == game->current_word[i]) 		&&
				(letter[1] == game->current_word[i + 1])	){
			game->word_progress[i] = game->current_word[i];
			game->word_progress[i + 1] = game->current_word[i + 1];
			i++;
			hitting++;
		}
	}
	
	if(hitting == 0)
		game->step_to_death++;
	
	if(game->step_to_death == 6)
		game->status = GAME_OVER;	
}

void game_letter_push_eng(game_stat_t *game, char letter){
	int hitting = 0;
	
	for(int i = 0; i < strlen(game->current_word); ++i){
		if(letter == game->current_word[i]){
			game->word_progress[i] = game->current_word[i];
			hitting++;
		}
	}
	
	if(hitting == 0)
		game->step_to_death++;
	
	if(game->step_to_death == 6)
		game->status = GAME_OVER;
}

static void game_word_progress_free(game_stat_t *game){
	if(game->word_progress != NULL)
		free(game->word_progress);
}

int game_reset(game_stat_t *game){
	game->current_word = str_vec_random(game->words_base);
	
	game_word_progress_free(game);
	int word_len = strlen(game->current_word) + 1;
	
	game->word_progress = malloc(word_len * sizeof(char));
	if(!game->word_progress){
		perror("Word_progress allocation failed");
		return 1;
	}
	
	memset(game->word_progress, '_', word_len * sizeof(char));
	game->word_progress[word_len - 1] = '\0';
	
	game->step_to_death = 0; 
	game->status = GAME_PROGRESS;
	
	return 0;
}

void game_free(game_stat_t *game){
	game_word_progress_free(game);
}

char *game_return_progress(game_stat_t *game){
	int size = strlen(game->current_word) + 1;
	
	static char *result;
	
	if(result != NULL)
		free(result);
		
	result = malloc(size * sizeof(char));
	
	memset(result, 0, size * sizeof(char));
	
	for(int i = 0, j = 0; j < size; i++, j++){
		result[i] = game->word_progress[j];
		if(game->word_progress[j] == '_')
			j++;
	}
	
	return result;
}

char *game_return_progress_eng (game_stat_t *game){
	return game->word_progress;
}

int	game_win_check(game_stat_t *game){
	return !strcmp(game->current_word, game->word_progress) ? 1 : 0;
}

int	game_lose_check(game_stat_t *game){
	return game->step_to_death == 6 ? 1 : 0;
}

int return_letter_by_keycode(unsigned int keycode, char* output){

	if(keycode == 0x18){
		strncpy(output, "й", 2 * sizeof(char));
	}else if(keycode == 0x19){
		strncpy(output, "ц", 2 * sizeof(char));
	}else if(keycode == 0x1a){
		strncpy(output, "у", 2 * sizeof(char));
	}else if(keycode == 0x1b){
		strncpy(output, "к", 2 * sizeof(char));
	}else if(keycode == 0x1c){
		strncpy(output, "е", 2 * sizeof(char));
	}else if(keycode == 0x1d){
		strncpy(output, "н", 2 * sizeof(char));
	}else if(keycode == 0x1e){
		strncpy(output, "г", 2 * sizeof(char));
	}else if(keycode == 0x1f){
		strncpy(output, "ш", 2 * sizeof(char));
	}else if(keycode == 0x20){
		strncpy(output, "щ", 2 * sizeof(char));
	}else if(keycode == 0x21){
		strncpy(output, "з", 2 * sizeof(char));
	}else if(keycode == 0x22){
		strncpy(output, "х", 2 * sizeof(char));
	}else if(keycode == 0x23){
		strncpy(output, "ъ", 2 * sizeof(char));
		
	}else if(keycode == 0x26){
		strncpy(output, "ф", 2 * sizeof(char));
	}else if(keycode == 0x27){
		strncpy(output, "ы", 2 * sizeof(char));
	}else if(keycode == 0x28){
		strncpy(output, "в", 2 * sizeof(char));
	}else if(keycode == 0x29){
		strncpy(output, "а", 2 * sizeof(char));
	}else if(keycode == 0x2a){
		strncpy(output, "п", 2 * sizeof(char));
	}else if(keycode == 0x2b){
		strncpy(output, "р", 2 * sizeof(char));
	}else if(keycode == 0x2c){
		strncpy(output, "о", 2 * sizeof(char));
	}else if(keycode == 0x2d){
		strncpy(output, "л", 2 * sizeof(char));
	}else if(keycode == 0x2e){
		strncpy(output, "д", 2 * sizeof(char));
	}else if(keycode == 0x2f){
		strncpy(output, "ж", 2 * sizeof(char));
	}else if(keycode == 0x30){
		strncpy(output, "э", 2 * sizeof(char));
		
	}else if(keycode == 0x34){
		strncpy(output, "я", 2 * sizeof(char));
	}else if(keycode == 0x35){
		strncpy(output, "ч", 2 * sizeof(char));
	}else if(keycode == 0x36){
		strncpy(output, "с", 2 * sizeof(char));
	}else if(keycode == 0x37){
		strncpy(output, "м", 2 * sizeof(char));
	}else if(keycode == 0x38){
		strncpy(output, "и", 2 * sizeof(char));
	}else if(keycode == 0x39){
		strncpy(output, "т", 2 * sizeof(char));
	}else if(keycode == 0x3a){
		strncpy(output, "ь", 2 * sizeof(char));
	}else if(keycode == 0x3b){
		strncpy(output, "б", 2 * sizeof(char));
	}else if(keycode == 0x3c){
		strncpy(output, "ю", 2 * sizeof(char));
	}else 
		return 1;
	return 0;
}

int return_letter_by_keycode_eng(unsigned int keycode, char* output){

	if(keycode == 0x18){
		strncpy(output, "q", sizeof(char));
	}else if(keycode == 0x19){
		strncpy(output, "w", sizeof(char));
	}else if(keycode == 0x1a){
		strncpy(output, "e", sizeof(char));
	}else if(keycode == 0x1b){
		strncpy(output, "r", sizeof(char));
	}else if(keycode == 0x1c){
		strncpy(output, "t", sizeof(char));
	}else if(keycode == 0x1d){
		strncpy(output, "y", sizeof(char));
	}else if(keycode == 0x1e){
		strncpy(output, "u", sizeof(char));
	}else if(keycode == 0x1f){
		strncpy(output, "i", sizeof(char));
	}else if(keycode == 0x20){
		strncpy(output, "o", sizeof(char));
	}else if(keycode == 0x21){
		strncpy(output, "p", sizeof(char));
		
	}else if(keycode == 0x26){
		strncpy(output, "a", sizeof(char));
	}else if(keycode == 0x27){
		strncpy(output, "s", sizeof(char));
	}else if(keycode == 0x28){
		strncpy(output, "d", sizeof(char));
	}else if(keycode == 0x29){
		strncpy(output, "f", sizeof(char));
	}else if(keycode == 0x2a){
		strncpy(output, "g", sizeof(char));
	}else if(keycode == 0x2b){
		strncpy(output, "h", sizeof(char));
	}else if(keycode == 0x2c){
		strncpy(output, "j", sizeof(char));
	}else if(keycode == 0x2d){
		strncpy(output, "k", sizeof(char));
	}else if(keycode == 0x2e){
		strncpy(output, "l", sizeof(char));
		
	}else if(keycode == 0x34){
		strncpy(output, "z", sizeof(char));
	}else if(keycode == 0x35){
		strncpy(output, "x", sizeof(char));
	}else if(keycode == 0x36){
		strncpy(output, "c", sizeof(char));
	}else if(keycode == 0x37){
		strncpy(output, "v", sizeof(char));
	}else if(keycode == 0x38){
		strncpy(output, "b", sizeof(char));
	}else if(keycode == 0x39){
		strncpy(output, "n", sizeof(char));
	}else if(keycode == 0x3a){
		strncpy(output, "m", sizeof(char));
	}else 
		return 1;
	return 0;
}

void game_draw(x_window_param_t *win, game_res_t *res, game_stat_t *game){
	XClearWindow(win->display, win->window);
	
	pixmap_attr_t *current_pixmap = &res->step_to_death[game->step_to_death];
	XCopyPlane(win->display, current_pixmap->bitmap, win->window, win->gc,
			 0, 0, current_pixmap->bitmap_width, current_pixmap->bitmap_height,
			 100, 0, 1);

#ifdef ENG_WORDS							
	char *word = game_return_progress_eng(game);
#else 
	char *word = game_return_progress(game);
#endif
	
	if(game_win_check(game)){
		XDrawString(win->display, win->window, win->gc, 50, 110,
			"You win!", strlen("You win!"));
		XDrawString(win->display, win->window, win->gc, 100, 220,
			game->current_word, strlen(game->current_word));
			
		printf("%s\n", game->current_word);
	} else if(game_lose_check(game)){
		XDrawString(win->display, win->window, win->gc, 40, 110,
			"You lose!", strlen("You lose!"));
		XDrawString(win->display, win->window, win->gc, 100, 220,
			game->current_word, strlen(game->current_word));
			
		printf("%s\n", game->current_word);
	} else{
		XDrawString(win->display, win->window, win->gc, 100, 220,
			word, strlen(word));
			
		printf("%s\n", word);
	}
	
	XFlush(win->display);
}

static int graphic_set(x_window_param_t *win){ 
/*	XFontStruct *fontInfo;
		
	if((fontInfo = XLoadQueryFont(win->display, "*cronyx*" )) == NULL){
		fprintf(stderr, "Font loading failed.\n");
		return 1;
	}
	
	XSetFont (win->display, win->gc, fontInfo->fid);
	*/
	
	XSetBackground(win->display, win->gc,
			WhitePixel(win->display, win->screen_number));
			
	XSetForeground(win->display, win->gc, 
			BlackPixel(win->display, win->screen_number) );
	
	return 0;
}

int pre_game_settings(x_window_param_t *win){
	if(graphic_set(win)){
		fprintf(stderr, "graphic_set failed.\n");
		return 1;
	}
}

static unsigned int event_keycode(x_window_param_t *win){
	return win->event.xkey.keycode;
}

void game_loop(x_window_param_t *win, game_res_t *res, game_stat_t *game){
	
	XNextEvent(win->display, &win->event);
	game_draw(win, res, game);
	
	while(win->display){
		XNextEvent(win->display, &win->event);
		
		if (win->event.type == KeyPress){
			
			if (event_keycode(win) == ESC_KEYCODE )
                break;
			
			if(game->status == GAME_OVER){
				game_reset(game);
			}else{
				
#ifdef ENG_WORDS
				char letter;
				return_letter_by_keycode_eng(event_keycode(win), &letter);
				
				game_letter_push_eng(game, letter);
#else
				char letter[3];
				letter[2] = '\0';
				return_letter_by_keycode(event_keycode(win), letter);
				
				game_letter_push(game, letter);
#endif
			}
			
			game_draw(win, res, game);
			
			if(game_win_check(game))
				game->status = GAME_OVER;
		}
	}
}

#ifdef ENG_WORDS
#undef ENG_WORDS
#endif
