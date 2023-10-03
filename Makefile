NAME        := libsockets.a

SRC_DIR     := src
SRCS        := $(wildcard $(SRC_DIR)/*.c)

INCS        := include

BUILD_DIR   := .build
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

CC          := gcc
CFLAGS      := -Wall -Wextra -Werror -DDEBUG -g -MMD -MP
CPPFLAGS    := $(addprefix -I,$(INCS))

AR          := ar
ARFLAGS     := -r -c -s -v


RM          := rm -f
DIR_DUP     = mkdir -p $(@D)


all: $(NAME)

$(NAME): $(OBJS)
	$(info Making static library : $@)
	$(AR) $(ARFLAGS) $(NAME) $(OBJS)


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(info Compiling object file : $@)
	$(DIR_DUP)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

-include $(DEPS)
	
clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(NAME)

.PHONY: clean
