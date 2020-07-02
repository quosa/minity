RM = rm -f
CXX = g++
CXXFLAGS = -std=c++14 -Wall -pedantic-errors -g -I /usr/local/include/SDL2
LDFLAGS = -L /usr/local/lib -l SDL2
SRCS = $(filter-out %.test.cpp, $(wildcard *.cpp))
OBJS = $(filter-out *.test.o, ${SRCS:.cpp=.o})
DEPS := ${SRCS:.cpp=.d}


TEST_SRCS = $(wildcard *.test.cpp)
TEST_OBJS = ${TEST_SRCS:.cpp=.o}
DEPS += ${TEST_SRCS:.cpp=.d}

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: clean

MAIN = minity
TEST = minitest

all: ${TEST} ${MAIN}
	@echo   Run ./${MAIN} to start minity

${TEST}: ${TEST_OBJS}
	${CXX} ${CXXFLAGS} ${TEST_OBJS} -o ${TEST} ${LDFLAGS} \
	&& ./${TEST} --reporter compact --success

${MAIN}: ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} -o ${MAIN} ${LDFLAGS}

%.o: %.cpp Makefile
	${CXX} ${CXXFLAGS} -MMD -MP -c $< -o $@

clean:
	${RM} ${MAIN} ${OBJS} test *.o *.d *~.

-include $(DEPS)

# from https://stackoverflow.com/questions/31093462/simple-makefile-for-c
# from https://stackoverflow.com/questions/52034997/how-to-make-makefile-recompile-when-a-header-file-is-changed
