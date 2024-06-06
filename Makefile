CFLAGS ?= -Wall -Og -g

libmiku.so: libmiku.c miku_title.h miku_data.h miku_data.o Makefile
	$(CC) $(CFLAGS) -fPIC -rdynamic -shared -ldl -o $@ libmiku.c miku_data.o

miku_title.h : miku_title.bmp
	xxd -i $< > $@

miku_data.o : miku_data.bin autodetect-arch
	objcopy -I binary -O ${FORMAT} -B ${ARCH} --rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@

detect: detect.c
	$(CC) -o detect detect.c

.PHONY: autodetect-arch
autodetect-arch: detect
	$(eval FORMAT=$(shell objdump -f ./detect | grep -oP 'file format \K.*'))
	$(eval ARCH=$(shell objdump -f ./detect | grep -oP 'architecture: \K[^,]*'))