TARGET=codec_demo
CC=gcc
CFLAGS=-I./include -L./libs -llvpopus -lopencore-amrwb -lopencore-amrnb -lvo-amrwbenc -no-pie

$(TARGET):codec_demo.c
	@$(CC) $< $(CFLAGS) -o $@

clean:
	@rm codec_demo

.PHONY: $(TARGET) clean
