# compiler variables and flags
CXX=g++
CC=gcc

CXXFLAGS=-Wall -Werror -g -std=c++11
CCFLAGS=-Wall -Werror -g -std=c99

RM=rm
RMFLAGS=-rf

# top-level directories
BUILD_DIR=build
INCL_DIR=include
SRC_DIR=src
TEST_DIR=test

# MAIN TARGETS AND VARIABLES

CXX_BUILD_DIR=$(BUILD_DIR)/cxx
CXX_OBJ_DIR=$(CXX_BUILD_DIR)/obj
CXX_BIN_DIR=$(CXX_BUILD_DIR)/bin

CC_BUILD_DIR=$(BUILD_DIR)/cc
CC_OBJ_DIR=$(CC_BUILD_DIR)/obj
CC_BIN_DIR=$(CC_BUILD_DIR)/bin

# files for main crypt
CXX_SRCS=$(SRC_DIR)/ciph.cpp
CXX_OBJS=$(patsubst $(SRC_DIR)/%.cpp, $(CXX_OBJ_DIR)/%.o, $(CXX_SRCS))

CC_SRCS=$(SRC_DIR)/main.c
CC_OBJS=$(patsubst $(SRC_DIR)/%.c, $(CC_OBJ_DIR)/%.o, $(CC_SRCS))

CXX_H=
CRYPT_H=$(INCL_DIR)/crypt.h $(wildcard $(INCL_DIR)/*.h)

# targets
CXX_TARGET=$(CXX_BIN_DIR)/ciph
CC_TARGET=$(CC_BIN_DIR)/main

all: $(CC_TARGET)

ciph: $(CXX_TARGET)
# COMPILE CXX
$(CXX_TARGET): $(CXX_OBJS) | $(CXX_BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
$(CXX_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(CXX_H) | $(CXX_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<
run-ciph:
	@./$(CXX_TARGET)

# CC TARGETS
# COMPILE CC
$(CC_TARGET): $(CC_OBJS) | $(CC_BIN_DIR)
	$(CC) $(CCFLAGS) -o $@ $^
$(CC_OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(CRYPT_H) | $(CC_OBJ_DIR)
	$(CC) $(CCFLAGS) -o $@ -c $<

run:
	@./$(CC_TARGET)
leaks:
	@leaks --atExit -- ./$(CC_TARGET)
debug:
	@lldb ./$(CC_TARGET)

# TEST TARGETS AND VARIABLES
TEST_BUILD_DIR=$(BUILD_DIR)/test
TEST_OBJ_DIR=$(TEST_BUILD_DIR)/obj
TEST_BIN_DIR=$(TEST_BUILD_DIR)/bin

TEST_SRCS=$(TEST_DIR)/main.cpp
TEST_OBJS=$(TEST_OBJ_DIR)/main.o
# dependencies for the test target (all test files in the test folder)
# $(wildcard $(INCL_DIR)/*.hpp)
TEST_DEPS=$(INCL_DIR)/ciph.h $(INCL_DIR)/bigint.h $(wildcard $(TEST_DIR)/*.hpp)

TEST_TARGET=$(TEST_BIN_DIR)/main

test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) | $(TEST_BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp $(TEST_DEPS) | $(TEST_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<
run-test:
	@./$(TEST_TARGET)
debug-test:
	@lldb ./$(TEST_TARGET)
leaks-test:
	@leaks --atExit -- ./$(TEST_TARGET)

# MISC
clean:
	$(RM) $(RMFLAGS) ./$(BUILD_DIR)

# create the .o and main executable directories as well as the
#  test binary directory if they don't exist.
$(BUILD_DIR) $(CXX_BUILD_DIR) $(CXX_OBJ_DIR) $(CXX_BIN_DIR) $(CC_BUILD_DIR) $(CC_OBJ_DIR) $(CC_BIN_DIR) $(TEST_BUILD_DIR) $(TEST_OBJ_DIR) $(TEST_BIN_DIR) $(INCL_DIR):
	@mkdir -p $@

.PHONY: clean all run leaks debug run-test leaks-test debug-test ciph run-ciph