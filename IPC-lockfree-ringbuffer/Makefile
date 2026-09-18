CXX = g++
CFLAGS = -O3 -std=c++17 -lrt -pthread

all :
	$(CXX) $(CFLAGS) main.cc ipc_ringbuffer.cc -o ipc_ringbuffer

clean:
	rm -rf ipc_ringbuffer

.PHONY:	all clean
