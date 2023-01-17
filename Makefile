RM = rm -f
# CXX = g++
CC = clang
CXX = clang++
#CFLAGS = -Wall -Wextra -pedantic-errors -Werror
CFLAGS =
#CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic-errors -Werror `sdl2-config --cflags`
CXXFLAGS = -std=c++17 `sdl2-config --cflags`
CPPFLAGS = -I /usr/local/include/SDL2 -I include -I ${IMGUI_DIR} -I external/metal-cpp
LDFLAGS = -L /usr/local/lib
LDLIBS = -l SDL2 -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework Foundation -framework QuartzCore -framework Metal `sdl2-config --libs`

# call with mode=release make all
ifeq ($(mode),release)
	CXXFLAGS += -O2
else
	mode = debug

	CXXFLAGS += -g -O0

ifdef coverage
	# coverage https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
	# SEE: tools/coverage/coverage.sh

	CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
	LDFLAGS += -fprofile-instr-generate

	# make clean test
	# LLVM_PROFILE_FILE="foo.profraw" ./bin/test
	# xcrun llvm-profdata merge -sparse foo.profraw -o foo.profdata
	# xcrun llvm-cov show ./bin/test -instr-profile=foo.profdata
	# xcrun llvm-cov report ./bin/test -instr-profile=foo.profdata
endif

	# profiling with clang/llvm
	# SEE: doc/profiling.md
	# SEE: tools/profile/profile.sh
	# CXXFLAGS += -g -O0 -fprofile-instr-generate -fcoverage-mapping
	# LDFLAGS += -fprofile-instr-generate


	# what takes so long?
	# CXXFLAGS += -g -O0 -v -ftime-report

	# clang does not support -pg profiling :-(
	# CXXFLAGS += -g -pg -O0

	# https://stackoverflow.com/questions/7408692/g-compiler-option-s-is-obsolete-and-being-ignored-c
	# obsolete? : LDFLAGS += -s
	# LDFLAGS += -pg
endif

# filter can be used to run only a subset of tests
# e.g. filter="detailed hand-written model with normals" make test
# See: https://github.com/catchorg/Catch2/blob/devel/docs/command-line.md#specifying-which-tests-to-run
ifndef filter
	filter = ""
endif

BIN_DIR = bin
BUILD_DIR = build
SOURCE_DIR = src
TEST_DIR = test
IMGUI_DIR = external/imgui

CXX_SRCS = $(wildcard ${SOURCE_DIR}/*.cpp)
CXX_SRCS += $(wildcard ${IMGUI_DIR}/*.cpp)
CXX_OBJS = $(patsubst ${IMGUI_DIR}/%, $(BUILD_DIR)/%, $(patsubst $(SOURCE_DIR)/%, $(BUILD_DIR)/%, ${CXX_SRCS:.cpp=.o}))
CXX_DEPS := $(patsubst ${IMGUI_DIR}/%,$(BUILD_DIR)/%, $(patsubst $(SOURCE_DIR)/%, $(BUILD_DIR)/%, ${CXX_SRCS:.cpp=.d}))

C_SRCS = $(wildcard ${SOURCE_DIR}/*.c)
C_OBJS = $(patsubst $(SOURCE_DIR)/%, $(BUILD_DIR)/%, ${C_SRCS:.c=.o})
C_DEPS := $(patsubst $(SOURCE_DIR)/%, $(BUILD_DIR)/%, ${C_SRCS:.c=.d})

SRCS = ${C_SRCS} ${CXX_SRCS}
OBJS = ${C_OBJS} ${CXX_OBJS}
DEPS = ${C_DEPS} ${CXX_DEPS}

TEST_INC = -I ${SOURCE_DIR}
TEST_SRCS = $(wildcard ${TEST_DIR}/*.cpp)
TEST_OBJS = $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/%,${TEST_SRCS:.cpp=.o})
DEPS += $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/%,${TEST_SRCS:.cpp=.d})

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: clean check coverage profile all

MAIN = minity
TEST = test

all: ${TEST} ${MAIN}
	@echo   Run ./${BIN_DIR}/${MAIN} to start minity

${MAIN}: ${OBJS}
	@echo building in $(mode) mode
	${CXX} ${LDFLAGS} ${OBJS} ${LDLIBS} -o ${BIN_DIR}/${MAIN}

-include $(DEPS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	${CC} ${CPPFLAGS} ${CFLAGS} -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(IMGUI_DIR)/%.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -MMD -MP -c $< -o $@

#
# Test Section
#
# No external libraries for tests!
# use filter="test case name" make test to run only a subset of tests
${TEST}: ${TEST_OBJS}
	@echo building in $(mode) mode
ifdef filter
	@echo using filter $(filter) for tests
endif
	${CXX} ${LDFLAGS} ${TEST_OBJS} -o ${BIN_DIR}/${TEST} \
	&& ./${BIN_DIR}/${TEST} --reporter compact --success "$(filter)"

# add src for private headers
$(BUILD_DIR)/%test.o: CPPFLAGS += ${TEST_INC}

# TODO: this is annoying duplication for tests
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -MMD -MP -c $< -o $@

clean:
	${RM} $(BUILD_DIR)/* $(BIN_DIR)/* *~.

#
# Linter Section
#
#    brew install bear # for compile_commands.json
#    brew install cppcheck

check: clean
	bear -- make minity
	@cppcheck --project=compile_commands.json --std=c++17 --language=c++ -i external --suppress=*:include* --suppress=*:external* --enable=style,unusedFunction,missingInclude

coverage: clean
	./tools/coverage/coverage.sh

profile: clean
	./tools/profile/profile.sh

# from https://stackoverflow.com/questions/31093462/simple-makefile-for-c
# from https://stackoverflow.com/questions/52034997/how-to-make-makefile-recompile-when-a-header-file-is-changed
