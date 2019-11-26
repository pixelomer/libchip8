DEBUG ?= 1
SOURCES := emulator.c main.c 
OBJECTS := $(SOURCES:%.c=out/%.o)
OUT = cc8
CFLAGS += -lncurses -lpthread -Werror -Wno-unused-command-line-argument

all: $(OUT)

clean:
	rm -f $(OUT)
	rm -f $(OBJECTS)

do: clean all

$(OUT): $(OBJECTS)
	$(CC) $(CFLAGS) -o "out/$@" $(OBJECTS:out/%.o="out/%.o")

out/%.o: %.c
	@mkdir -p "$$(dirname "$@")"
	$(CC) $(CFLAGS) -c "$<" -o "$@"
