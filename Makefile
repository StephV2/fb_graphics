CC := clang -Wall -Wextra
FILES := impl.c fb.c
DEPS := fb.h $(FILES)
TARGET := bin/impl

.PHONY: all debug

all: $(TARGET)
debug: $(TARGET)_d
asan: $(TARGET)_asan

$(TARGET): $(DEPS)
	$(CC) -O3 -o $@ $(FILES)
$(TARGET)_d: $(DEPS)
	$(CC) -DDEBUG -g -o $@ $(FILES)
$(TARGET)_asan: $(DEPS)
	$(CC) -fsanitize=address -DDEBUG -g -o $@ $(FILES)
