CC = gcc
CFLAGS = -Wall -Wextra

TARGET = main
SERVER_SRC = server.c
C_TERM_SRC = c-term.c
C_DISP_SRC = c-disp.c

C_TERM_OUT = c1-term.out c2-term.out c3-term.out c4-term.out
C_DISP_OUT = c1-disp.out c2-disp.out c3-disp.out c4-disp.out

.PHONY: all clean

all: $(TARGET)

$(TARGET): server c-term c-disp

server: $(SERVER_SRC)
	$(CC) $(CFLAGS) -o ./server.out $(SERVER_SRC)

c-term: $(C_TERM_SRC)
	$(CC) $(CFLAGS) -o ./c1-term.out -DC1 $(C_TERM_SRC)
	$(CC) $(CFLAGS) -o ./c2-term.out -DC2 $(C_TERM_SRC)
	$(CC) $(CFLAGS) -o ./c3-term.out -DC3 $(C_TERM_SRC)
	$(CC) $(CFLAGS) -o ./c4-term.out -DC4 $(C_TERM_SRC)

c-disp: $(C_DISP_SRC)
	$(CC) $(CFLAGS) -o ./c1-disp.out -DC1 $(C_DISP_SRC)
	$(CC) $(CFLAGS) -o ./c2-disp.out -DC2 $(C_DISP_SRC)
	$(CC) $(CFLAGS) -o ./c3-disp.out -DC3 $(C_DISP_SRC)
	$(CC) $(CFLAGS) -o ./c4-disp.out -DC4 $(C_DISP_SRC)

clean:
	rm -f server
	rm -f $(C_TERM_OUT)
	rm -f $(C_DISP_OUT)
