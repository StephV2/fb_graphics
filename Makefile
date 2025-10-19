CC := clang -Wall -Wextra
FILES := impl.c fb.c
DEPS := fb.h
TARGET := bin/impl

.PHONY: all debug

all: $(TARGET)
debug: $(TARGET)_d

$(TARGET): $(FILES) $(DEPS)
	$(CC) -O3 -o $@ $(FILES)
$(TARGET)_d: $(FILES) $(DEPS)
	$(CC) -DDEBUG -g -o $@ $(FILES)
