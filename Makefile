EXE_NAME = main

INC_DIR = include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

EXE = $(BIN_DIR)/$(EXE_NAME)
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)


CPPFLAGS = -Iinclude -MMD -MP -Ofast -DERROR_CHECKING
CFLAGS   = -Wall -Wextra -Werror -g
LDFLAGS  = $(foreach d, $(LIB_DIRS), -L $d/bin)
LDLIBS   = -lpthread $(foreach d, $(DEPS), -l$d)
INCLUDES = $(foreach d, $(LIB_INCLUDES), -I$d)

.PHONY: all clean  fclean re
all: $(LIBSALL) $(EXE) 

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ $(INCLUDES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c $< -o $@ 

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean: $(LIBSSCLEAN)
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

fclean: clean
	rm -f $(EXE)

re: fclean | $(EXE)

%clean: %
	$(MAKE) -C $< clean

%all: %
	$(MAKE) -C $< all

-include $(OBJ:.o=.d)