
LDFLAGS=-lSDL

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
OUTPUT = gpt

all:$(SRCS)
	gcc -o $(OUTPUT) $(SRCS) $(LDFLAGS)

clean:
	rm -rf $(OBJS) $(OUTPUT)

