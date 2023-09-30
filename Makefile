NAME        := libsockets.a

BUILD_DIR   := .build
SRCS        := sockets.c 

SRCS        := $(SRCS:%=$(SRC_DIR)/%)
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

CC          := gcc
CFLAGS      := -Wall -Wextra -Werror -DDEBUG -g
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

clean:
	$(RM) $(OBJS) $(DEPS)

fclean: clean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: clean fclean re
