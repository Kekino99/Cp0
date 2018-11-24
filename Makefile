####
#
#
####

ROOT_DIR ?= .
TARGET := sudoku

C_SRCS := $(wildcard ./*.c) # $(shell find $(ROOT_DIR) -name "*.c")
C_HDRS := $(wildcard ./*.c) # $(shell find $(ROOT_DIR) -name "*.h")
C_OBJS := $(C_SRCS:.c=.o)

ifeq ($(OS),Windows_NT)

else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		PREBUILD_OBJS := teacher_linux.o
	endif
	ifeq ($(UNAME_S),Darwin)
		# PREBUILD_OBJS := teacher_osx.o
	endif
endif

OBJS_DIR := $(ROOT_DIR)/objs
OBJS_FILES := $(addprefix $(OBJS_DIR)/, $(C_OBJS))

C_WFLAGS := -Wall -Wextra  # -Werror
C_IFLAGS := -I$(ROOT_DIR)

CFLAGS ?= -O0 -g
#LDFLAGS ?=

CC = gcc

# special rules
.PHONY: default clean mkdir-debug mkdir-release

# default
default: $(TARGET)

$(TARGET): $(OBJS_FILES)
	@echo "Linking: $@"
	@$(CC) $(LDFLAGS) -o $@ $^ $(PREBUILD_OBJS)

# build rules
$(OBJS_DIR)/%.o: %.c $(C_HDRS)
	@echo "Compiling: $< -> $@"
	@echo "$(OBJS_FILES)"
	@mkdir -p $(dir $@)
	@$(CC) $(C_WFLAGS) $(C_IFLAGS) $(CFLAGS) -c -o $@ $<

# utility rules
clean:
	@echo "Cleaning object files"
	@$(RM) -v $(OBJS_FILES)
	@echo "Cleaning binaries"
	@$(RM) -v $(TARGET)

mkdir-debug:
	@mkdir -p $(DGGA_DEBUG_OBJ_DIR)

mkdir-release:
	@mkdir -p $(DGGA_RELEASE_OBJ_DIR)
