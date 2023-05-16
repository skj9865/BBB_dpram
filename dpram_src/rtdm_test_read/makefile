XENO_CONFIG := /usr/xenomai/bin/xeno-config
SKIN=alchemy
MAIN_SRC=rtdm_test_read
TARGET=rtdm_test_read

LM=-lm

CFLAGS := $(shell $(XENO_CONFIG) --skin=$(SKIN) --cflags)
LDFLAGS := $(LM) $(shell $(XENO_CONFIG) --skin=$(SKIN) --ldflags)
CC := $(shell $(XENO_CONFIG) --cc)

$(TARGET): $(MAIN_SRC).c
	$(CC) -o $@.out $< $(CFLAGS) $(LDFLAGS)

.PHONY : clean
clean:
	rm *.out
