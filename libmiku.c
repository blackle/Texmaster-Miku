#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include "miku_title.h"
#include "miku_data.h"
#include <time.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>

typedef FILE* (*fopen_fun_ptr)(const char *, const char *);

__attribute__((constructor)) void seed_rng() {
	srand(getpid() + time(NULL));
}

FILE *fopen(const char *pathname, const char *mode) {
	static fopen_fun_ptr orig_fopen = NULL;
	if (orig_fopen == NULL) orig_fopen = (fopen_fun_ptr)dlsym(RTLD_NEXT, "fopen");

	if (strcmp(pathname, "Texmaster2009.ubuntu10.04.ini") == 0) {
		return orig_fopen("Texmaster2009.ubuntu10.04.miku.ini", mode);
	} else if (strcmp(pathname, "data/bmp/title.bmp") == 0) {
		return fmemopen(miku_title_bmp, (size_t)miku_title_bmp_len, mode);
	} else {
		return orig_fopen(pathname, mode);
	}
}

typedef int (*upper_blit_ptr)(SDL_Surface*, const SDL_Rect*, SDL_Surface*, SDL_Rect*);

int SDL_UpperBlit(SDL_Surface * src, const SDL_Rect * srcrect, SDL_Surface * dst, SDL_Rect * dstrect) {
	static upper_blit_ptr orig_blit = NULL;
	if (orig_blit == NULL) orig_blit = (upper_blit_ptr)dlsym(RTLD_NEXT, "SDL_UpperBlit");

	static SDL_Surface* last_src = NULL;
	static SDL_Surface* last_src_replacement = NULL;
	// replace background images
	if (src && src->w == 320 && src->h == 240) {
		if (last_src != src) {
			if (last_src_replacement != NULL) {
				SDL_FreeSurface(last_src_replacement);
			}
			unsigned int random = rand() % miku_img_count;
			const char* mem = (const char*)&_binary_miku_data_bin_start + miku_img_size*random;
			SDL_RWops* mydata = SDL_RWFromConstMem(mem, (int)miku_img_size);
			last_src_replacement = SDL_LoadBMP_RW(mydata, 0);
			SDL_FreeRW(mydata);
			last_src = src;
		}
		return orig_blit(last_src_replacement, srcrect, dst, dstrect);
	}
	return orig_blit(src, srcrect, dst, dstrect);
}