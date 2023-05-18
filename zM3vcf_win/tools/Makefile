CC = gcc
CFLAGS = -Wall -O0 -g
TARGET = pVcf.exe gVMH.exe vCM.exe readVcfTest.exe 
VCF_LIB_DIR = ../vcflib/bin

all: $(TARGET)

pVcf.exe:copyCreateVcf.c 
	$(CC) $^ -o $@ $(CFLAGS) -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -lpthread -l:libvcf.a -fopenmp -lz

gVMH.exe: getVcfMH.c
	$(CC) $^ -o $@ $(CFLAGS) -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -lpthread -l:libvcf.a -fopenmp -lz

vCM.exe: vcfCutMarkers.c
	$(CC) $^ -o $@ $(CFLAGS) -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -lpthread -l:libvcf.a -fopenmp -lz
	
readVcfTest.exe: readVcfTest.c
	$(CC) $^ -o $@ $(CFLAGS) -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -lpthread -l:libvcf.a -fopenmp -lz
	
.PHONY:clean
clean:
	@del /q $(TARGET) 2>nul
	