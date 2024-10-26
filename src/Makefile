CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

PROJECT_SRC = $(wildcard *.cpp)
TEST_SRC = $(wildcard test/*.cpp)
PROJECT_OBJ = $(PROJECT_SRC:.cpp=.o)
TEST_OBJ = $(TEST_SRC:.cpp=.o)
OBJ = $(PROJECT_OBJ) $(TEST_OBJ)

TEST_EXEC = test_exec

all: $(TEST_EXEC)

$(TEST_EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(OBJ) -o $(TEST_EXEC)

clean:
	rm -f $(PROJECT_OBJ) $(TEST_OBJ) $(TEST_EXEC) **/*.gcda **/*.gcno coverage.info
	rm -rf coverage

container/%.o: container/%.cpp
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) -c $< -o $@

test/%.o: test/%.cpp
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) -c $< -o $@

style:
	clang-format -n -style=Google $(wildcard **/*.h **/*.tpp **/*.cpp)

fix_style:
	clang-format -i -style=Google $(wildcard ./**/*.h ./**/*.tpp ./**/*.cpp)

valgrind: $(TEST_EXEC)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_EXEC)
