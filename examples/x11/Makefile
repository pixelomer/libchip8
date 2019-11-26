CFLAGS = -lX11 -lpthread -Wno-unused-command-line-argument
OUT_DIR = out
OUT_NAME = $(OUT_DIR)/chip8
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:%.c=$(OUT_DIR)/%.o)
XCODE_PATH = /Applications/Xcode.app
MACOS_FLAGS = -I/opt/X11/include -L/opt/X11/lib -I.
MACOS_REQ_FOLDERS = /opt/X11 $(XCODE_PATH)
OS_READABLE = Unknown
ifneq ($(OS),Windows_NT)
ifeq ($(shell uname -s),Darwin)
OS_READABLE = macOS
CFLAGS += $(MACOS_FLAGS)
endif
endif

all: check-depends $(OUT_NAME)

check-depends:
	@echo "==> Checking requirements..."
ifeq ($(OS_READABLE),macOS)
	@for path in $(MACOS_REQ_FOLDERS); do \
		if [ ! -d $${path} ]; then \
			@echo "[!] Missing directory: $${path}"; \
		fi; \
	done;
endif
	@if [ ! -d $(OUT_DIR) ]; then \
		c="mkdir -p $(OUT_DIR)"; \
		printf "==> Creating output folder...\n$${c}\n"; \
		eval $${c}; \
	fi

$(OUT_NAME): $(OBJECTS)
	@echo "==> Producing executable..."
	$(CC) $(CFLAGS) -o "$@" $(OBJECTS)

$(OUT_DIR)/%.o: %.c
	@mkdir -p $(OUT_DIR)
	@echo "==> Compiling $<..."
	$(CC) $(CFLAGS) -c "$<" -o "$@"