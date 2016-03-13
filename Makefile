
CUNIT_NAME=CUnit-2.1-3
CUNIT_ARCHIVE=$(CUNIT_NAME).tar.bz2
CUNIT_DOWNLOAD_URL=http://downloads.sourceforge.net/project/cunit/CUnit/2.1-3/$(CUNIT_ARCHIVE)
CUNIT_LOCAL_ARCHIVE=$(CUNIT_ARCHIVE)
CUNIT_PATH=$(CUNIT_NAME)

PREPROC=-D_GNU_SOURCE=1
INCLUDE_PATH=./include
CFLAGS=-I$(INCLUDE_PATH) $(PREPROC)
SRC_PATH=src
OBJ_PATH=src
LIBS=-lcunit

SOURCES= $(SRC_PATH)/cpu.c $(SRC_PATH)/cpu_test.c $(SRC_PATH)/ram.c \
			$(SRC_PATH)/cpu_shared.c

OBJS=$(OBJ_PATH)/cpu_test.o $(OBJ_PATH)/cpu_shared.o $(OBJ_PATH)/cpu.o \
			$(OBJ_PATH)/ram.o
	

TARGETS=cpu_test

.PHONY: all clean init deps

all: $(UNIT_TESTS)

CC=gcc

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	@rm -f $(SRC_PATH)*.d $(OBJ_PATH)*.o $(TARGETS)

deps: $($(SRC_PATH)/SOURCES:.c=.d)

init:
	@apt-get install -y libcunit1-dev

include $(SOURCES:.c=.d)

cpu_test: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

