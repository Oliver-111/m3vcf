CC = gcc

INC_DIR = inc
SRC_DIR = src
OBJ_DIR	= obj
TEST_DIR = tst
TARGET_DIR = bin


SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst %c, $(OBJ_DIR)/%o,$(notdir $(SRCS)))
LIBNAME = libvcf
LIBHEADFILE = vcflib.h
CFLAGS = -Wall -O0 -g -fopenmp -lz -I $(INC_DIR)

all:$(LIBNAME).a $(LIBNAME).so
	copy $(INC_DIR)\$(LIBHEADFILE) $(TARGET_DIR)\$(LIBHEADFILE) 
		
$(LIBNAME).a: $(OBJS)
	ar -rsv $(TARGET_DIR)/$@ $^
	
	
$(LIBNAME).so:$(SRCS)
	$(CC) -fpic -shared -o $(TARGET_DIR)/$@ $^ $(CFLAGS) 
	
$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS) 

.PHONY:clean
clean:
	@ del /q $(TARGET_DIR)
	@ del /q $(OBJ_DIR)
	@ echo.
	@ echo clean done
	@ echo.