CC = gcc
VCF_LIB_DIR = ..\bin

CFLAGS = -Wall -O0 -g -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -l:libvcf.a -fopenmp -lz

simpleRead.exe:simpleRead.c
	$(CC) $^ -o $@ $(CFLAGS)
.PHONY:clean 
clean:
	@ del /q simpleRead.exe
	@ echo.
	@ echo clean done
	@ echo.