#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/mman.h>
#include <errno.h>

#include "miku_title.h"
#include "miku_data_jxl.h"

#define J40_IMPLEMENTATION
#define J40_CONFIRM_THAT_THIS_IS_EXPERIMENTAL_AND_POTENTIALLY_UNSAFE
#include "j40.h"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>

typedef FILE* (*fopen_fun_ptr)(const char *, const char *);

__attribute__((constructor)) void seed_rng() {
	srand(getpid() + time(NULL));
}

// advise that we're gonna be randomly accessing our miku dataset
// this is overkill, but it is fun overkill :3
__attribute__((constructor)) void advise_random() {
	// 0x7fffe56580ad-0x7fffe5658000
	void* start = (void*)&_binary_miku_data_jxl_bin_start - (((long long int)&_binary_miku_data_jxl_bin_start) & 0x7ff); //round down to page offset(hopefully)
	void* end = (void*)&_binary_miku_data_jxl_bin_end;
	int res = madvise(start, (size_t)(end - start), MADV_RANDOM);
	if (res != 0) {
		printf("couldn't madvise - %s\n", strerror(errno));
	}
}

FILE *fopen(const char *pathname, const char *mode) {
	static fopen_fun_ptr orig_fopen = NULL;
	if (orig_fopen == NULL) orig_fopen = (fopen_fun_ptr)dlsym(RTLD_NEXT, "fopen");

	if (strcmp(pathname, "data/bmp/title.bmp") == 0) {
		return fmemopen(miku_title_bmp, (size_t)miku_title_bmp_len, mode);
	} else {
		return orig_fopen(pathname, mode);
	}
}

typedef int (*upper_blit_ptr)(SDL_Surface*, const SDL_Rect*, SDL_Surface*, SDL_Rect*);

#define IMG_WIDTH 320
#define IMG_HEIGHT 240

int SDL_UpperBlit(SDL_Surface * src, const SDL_Rect * srcrect, SDL_Surface * dst, SDL_Rect * dstrect) {
	static upper_blit_ptr orig_blit = NULL;
	if (orig_blit == NULL) orig_blit = (upper_blit_ptr)dlsym(RTLD_NEXT, "SDL_UpperBlit");

	static SDL_Surface* last_src = NULL;
	static SDL_Surface* last_src_replacement = NULL;
	// replace background images
	if (src && src->w == IMG_WIDTH && src->h == IMG_HEIGHT) {
		if (last_src != src) {
			unsigned int random = rand() % miku_image_count;
			int offset = 0;
			for (int k = 0; k < random; k++) {
				offset += miku_image_sizes[k];
			}
			int size = miku_image_sizes[random];

			j40_image image;
			const char* mem = (const char*)&_binary_miku_data_jxl_bin_start + offset;
			j40_from_memory(&image, (void*)mem, size, NULL);
			j40_output_format(&image, J40_RGBA, J40_U8X4);

			if (j40_next_frame(&image)) {
				j40_frame frame = j40_current_frame(&image);
				j40_pixels_u8x4 pixels = j40_frame_pixels_u8x4(&frame, J40_RGBA);
				if (last_src_replacement != NULL) {
					SDL_FreeSurface(last_src_replacement);
				}
				last_src_replacement = SDL_CreateRGBSurface(0, IMG_WIDTH, IMG_HEIGHT, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
				// printf("%d\n", last_src_replacement->pitch);
				if (SDL_LockSurface(last_src_replacement) != 0) {
					printf("err: %s\n", SDL_GetError());
				}
				for (int y = 0; y < IMG_HEIGHT; ++y) {
					memcpy(last_src_replacement->pixels + y*IMG_WIDTH*4, j40_row_u8x4(pixels, y), 4*IMG_WIDTH);
				}
				SDL_UnlockSurface(last_src_replacement);

			} else {
				printf("fuck! %d\n", random);
			}
			j40_free(&image);

			// SDL_RWops* mydata = SDL_RWFromConstMem(mem, img_size);
			// last_src_replacement = SDL_LoadBMP_RW(mydata, 0);
			// SDL_FreeRW(mydata);
			last_src = src;
		}
		return orig_blit(last_src_replacement, srcrect, dst, dstrect);
	}
	return orig_blit(src, srcrect, dst, dstrect);
}

typedef int (*setcap_ptr)(const char*, const char*);

void SDL_WM_SetCaption(const char *title, const char *icon) {
	setcap_ptr original_setcap = (setcap_ptr)dlsym(RTLD_NEXT, "SDL_WM_SetCaption");
	original_setcap("Texmaster 2009: Hatsune Miku Edition", icon);
}