CC = gcc
M3VCF_SRC_DIR = src/m3vcf
VCF_LIB_DIR = vcflib/bin
SRCS = $(wildcard $(M3VCF_SRC_DIR)/*.c)
OBJS = $(patsubst %c, %o, $(SRCS))
COMMOND_LINE = zM3vcf
CL_OBJ = src/main.o
INTERFACE_TEST = m3vcfTest
IT_OBJ = src/m3vcfTest.o
CFLAGS = -Wall -O0 -g -I $(VCF_LIB_DIR) -L $(VCF_LIB_DIR) -lpthread -l:libvcf.a -fopenmp -lz

all: $(COMMOND_LINE) $(INTERFACE_TEST)
	@echo
	@echo "$(COMMOND_LINE) done!!!"
	@echo "$(INTERFACE_TEST) done!!!"
	@echo


$(COMMOND_LINE): $(OBJS) $(CL_OBJ)
	$(CC) $^ -o $@ $(CFLAGS)
	
$(INTERFACE_TEST): $(OBJS) $(IT_OBJ)
	$(CC) $^ -o $@ $(CFLAGS)

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS) 
	
.PHONY:clean vcflib

clean:
	rm -rf $(OBJS) $(IT_OBJ) $(CL_OBJ) $(COMMOND_LINE) $(INTERFACE_TEST)
vcflib:
	@cd vcflib && make clean
	@cd vcflib && make
	@echo
	@echo "libvcf done!!!"
	@echo
