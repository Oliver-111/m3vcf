CC = gcc
CFLAGS = -Wall -O0 -g
TARGET = pVcf gVMH vCM readVcfTest 
VCF_LIB_DIR = ../vcflib/bin

all: $(TARGET)

pVcf:copyCreateVcf.c 
	$(CC) $^ -o $@ $(CFLAGS) -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -lpthread -l:libvcf.a -fopenmp -lz

gVMH: getVcfMH.c
	$(CC) $^ -o $@ $(CFLAGS) -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -lpthread -l:libvcf.a -fopenmp -lz

vCM: vcfCutMarkers.c
	$(CC) $^ -o $@ $(CFLAGS) -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -lpthread -l:libvcf.a -fopenmp -lz
	
readVcfTest: readVcfTest.c
	$(CC) $^ -o $@ $(CFLAGS) -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -lpthread -l:libvcf.a -fopenmp -lz
	
.PHONY:clean
clean:
	rm -rf $(TARGET) *.o
