NAME        := libsockets.a

BUILD_DIR   := .build
SRCS        := sockets.c 

SRCS        := $(SRCS:%=$(SRC_DIR)/%)
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

CC          := gcc
CFLAGS      := -Wall -Wextra -Werror -DDEBUG -g -MMD -MP
AR          := ar
ARFLAGS     := -r -c -s


RM          := rm -f
DIR_DUP     = mkdir -p $(@D)


all: $(NAME)

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) $(NAME) $(OBJS)
	$(info CREATED $@)


$(BUILD_DIR)/%.o: %.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) -c $< -o $@
	$(info CREATED $@)

-include $(DEPS)

docs:
	doxygen
	$(MAKE) -C DOCS/latex

docs-rm:
	rm -rf DOCS
	
clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(NAME)

.PHONY: clean
