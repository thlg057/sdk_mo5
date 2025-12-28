CC = cmoc
AR = lwar
# Utilisation de --compile à la place de -c pour plus de clarté avec CMOC
CFLAGS = --thommo --compile -Iinclude
ARFLAGS = -c

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
LIB_DIR = lib
DIST_DIR = sdk_mo5

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
HDRS = $(wildcard $(INC_DIR)/*.h)
LIB_NAME = libmo5.a

all: $(LIB_DIR)/$(LIB_NAME)

$(LIB_DIR)/$(LIB_NAME): $(OBJS)
	@mkdir -p $(LIB_DIR)
	@echo "Archivage : $@"
	$(AR) $(ARFLAGS) $@ $(OBJS)

# Changement de l'ordre ici : on place le fichier source à la fin
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HDRS)
	@mkdir -p $(OBJ_DIR)
	@echo "Compilation : $<"
	$(CC) $(CFLAGS) -o $@ $<

export_sdk: all
	@mkdir -p $(DIST_DIR)/include
	@mkdir -p $(DIST_DIR)/lib
	cp $(INC_DIR)/*.h $(DIST_DIR)/include/
	cp $(LIB_DIR)/$(LIB_NAME) $(DIST_DIR)/lib/
	@echo "SDK exporté dans /$(DIST_DIR)"

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(DIST_DIR)

.PHONY: all clean export_sdk