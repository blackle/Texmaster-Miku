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

typedef FILE* (*fopen_fun_ptr)(const char *, const char *);

__attribute__((constructor)) void seed_rng() {
	srand(getpid() + time(NULL));
}

bool is_bg(const char* pathname) {
	if (strlen(pathname) != 14) {
		return false;
	}
	if (memcmp(pathname, "data/bg_", 8)) {
		return false;
	}
	if (memcmp(pathname+10, ".bmp", 4)) {
		return false;
	}
	return true;
}

FILE *fopen(const char *pathname, const char *mode) {
	fopen_fun_ptr orig_fopen = (fopen_fun_ptr)dlsym(RTLD_NEXT, "fopen");
	if (is_bg(pathname)) {
		unsigned int random = rand() % miku_img_count;
		// void* img = malloc(miku_img_size);
		// memcpy(img, , (size_t)miku_img_size);
		return fmemopen((char*)&_binary_miku_data_bin_start + miku_img_size*random, (size_t)miku_img_size, mode);
	} else if (strcmp(pathname, "Texmaster2009.ubuntu10.04.ini") == 0) {
		return orig_fopen("Texmaster2009.ubuntu10.04.miku.ini", mode);
	} else if (strcmp(pathname, "data/bmp/title.bmp") == 0) {
		return fmemopen(miku_title_bmp, (size_t)miku_title_bmp_len, mode);
	} else {
		return orig_fopen(pathname, mode);
	}
}
