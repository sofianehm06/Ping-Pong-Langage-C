# Makefile pour le mini-jeu Tennis de Table
# Usage : make | make run | make clean

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
SRC     = tennis_de_table.c
BIN     = tennis_de_table

# Sous Windows, l'executable porte l'extension .exe
ifeq ($(OS),Windows_NT)
    BIN := $(BIN).exe
endif

# Cible par defaut : compiler le jeu.
$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

# Compiler (si besoin) puis lancer le jeu.
run: $(BIN)
	./$(BIN)

# Supprimer l'executable.
clean:
	rm -f $(BIN)

.PHONY: run clean
