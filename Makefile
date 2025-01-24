SOURCES = ping.c main.c parse.c util.c
OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(SOURCES:.c=.o))

CFLAGS = #-Wall -Wextra -Werror -fsanitize=address #-g

NAME = ft_ping

LIBFT_DIR = libft/
LIBFT = $(LIBFT_DIR)libft.a

all: $(NAME)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	cc $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ) $(LIBFT)
	cc $(OBJ) $(CFLAGS) $(LIBFT) -o $(NAME)

$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR) bonus

clean:
	$(MAKE) -C $(LIBFT_DIR) clean
	@rm -rf $(OBJ_DIR)

fclean: clean
	$(MAKE) -C $(LIBFT_DIR) fclean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
