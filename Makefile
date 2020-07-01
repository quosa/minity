RM = rm -f
CXX = g++
CXXFLAGS = -std=c++0x -Wall -pedantic-errors -g -I /usr/local/include/SDL2
LDFLAGS = -L /usr/local/lib -l SDL2
SRCS = $(wildcard *.cpp)
OBJS = ${SRCS:.cpp=.o}
DEPS := ${SRCS:.cpp=.d}

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: clean

MAIN = full

all: ${MAIN}
	@echo   Run ./${MAIN} to start full3D

${MAIN}: ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} -o ${MAIN} ${LDFLAGS}

%.o: %.cpp Makefile
	${CXX} ${CXXFLAGS} -MMD -MP -c $< -o $@

clean:
	${RM} ${MAIN} ${OBJS} *.o *~.

-include $(DEPS)

# from https://stackoverflow.com/questions/31093462/simple-makefile-for-c
# from https://stackoverflow.com/questions/52034997/how-to-make-makefile-recompile-when-a-header-file-is-changed
