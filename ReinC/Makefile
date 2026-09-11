CC = cc
CFLAGS = -O2 -Wall -Wextra -std=c11
LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

BIN = snake_train

all: $(BIN)

$(BIN): env.c renderer.c renderer.h model.c autograd.c autograd.h \
        mat.c mat.h arena.c arena.h prng.c prng.h base.h
	$(CC) $(CFLAGS) env.c -o $(BIN) $(LDLIBS)

clean:
	rm -f $(BIN)

.PHONY: all clean
