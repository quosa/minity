RM = rm -f
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -pedantic-errors -Werror
CPPFLAGS = -I /usr/local/include/SDL2 -I include
LDFLAGS = -L /usr/local/lib
LDLIBS = -l SDL2

ifeq ($(mode),release)
	CXXFLAGS += -O2
else
	mode = debug
	CXXFLAGS += -g -O0
	# https://stackoverflow.com/questions/7408692/g-compiler-option-s-is-obsolete-and-being-ignored-c
	# obsolete? : LDFLAGS += -s
endif

BIN_DIR = bin
BUILD_DIR = build
SOURCE_DIR = src
TEST_DIR = test

SRCS = $(wildcard ${SOURCE_DIR}/*.cpp)
OBJS = $(patsubst $(SOURCE_DIR)/%,$(BUILD_DIR)/%,${SRCS:.cpp=.o})
DEPS := ${SRCS:.cpp=.d}

TEST_INC = -I ${SOURCE_DIR}
TEST_SRCS = $(wildcard ${TEST_DIR}/*.cpp)
TEST_OBJS = $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/%,${TEST_SRCS:.cpp=.o})
DEPS += ${TEST_SRCS:.cpp=.d}

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: clean

MAIN = minity
TEST = test

all: ${TEST} ${MAIN}
	@echo   Run ./${BIN_DIR}/${MAIN} to start minity

# No external libraries for tests!
${TEST}: ${TEST_OBJS}
	@echo building in $(mode) mode
	${CXX} ${TEST_OBJS} -o ${BIN_DIR}/${TEST} \
	&& ./${BIN_DIR}/${TEST} --reporter compact --success

${MAIN}: ${OBJS}
	@echo building in $(mode) mode
	${CXX} ${LDFLAGS} ${OBJS} ${LDLIBS} -o ${BIN_DIR}/${MAIN}

# add src for private headers
$(BUILD_DIR)/%test.o: CPPFLAGS += ${TEST_INC}

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -MMD -MP -c $< -o $@

# TODO: this is annoying duplication for tests
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -MMD -MP -c $< -o $@

clean:
	${RM} $(BUILD_DIR)/* $(BIN_DIR)/* *~.

-include $(DEPS)

# from https://stackoverflow.com/questions/31093462/simple-makefile-for-c
# from https://stackoverflow.com/questions/52034997/how-to-make-makefile-recompile-when-a-header-file-is-changed
