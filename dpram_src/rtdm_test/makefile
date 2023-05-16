XENO_CONFIG := /usr/xenomai/bin/xeno-config
SKIN=alchemy
MAIN_SRC=rtdm_test
TARGET=rtdm_test

LM=-lm

CFLAGS := $(shell $(XENO_CONFIG) --skin=$(SKIN) --cflags)
LDFLAGS := $(LM) $(shell $(XENO_CONFIG) --skin=$(SKIN) --ldflags)
CC := $(shell $(XENO_CONFIG) --cc)

$(TARGET): $(MAIN_SRC).c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

