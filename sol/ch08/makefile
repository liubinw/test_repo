
LDFLAGS=-lSDL

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
OUTPUT = gpt

all:$(SRCS)
	g++ -o $(OUTPUT) $(SRCS) $(LDFLAGS)

clean:
	rm -rf $(OBJS) $(OUTPUT)

