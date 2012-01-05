CC = g++
exes = ./wave

CPPFLAGS = -g -Wall

src = $(@shell ls \*.cpp)
objs = $(src:.cpp=.o)

.PHONY: all
all: $(exes)

.PHONY: clean
clean:
	@rm -f $(objs) $(exes)

$(exes): $(objs)

%.o: %.cpp %.h
	$(COMPILE.cpp) -o $@ $< 



