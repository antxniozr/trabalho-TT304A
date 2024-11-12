# Variáveis para simplificar a escrita dos comandos
CC = gcc
CFLAGS = -Wall -pthread
TARGET = mergesort_program
SRC = mergesort.c
# Alvo padrão: compila o programa
all: $(TARGET)
# Alvo que compila o executável
$(TARGET): $(SRC)   $(CC) $(CFLAGS) -o $(TARGET) $(SRC)
# Alvo de limpeza que remove o executável
clean:  rm -f $(TARGET)
