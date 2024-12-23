TARGET = sudoku

CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c11 -g
OS = $(shell uname)
MODULES = $(wildcard modules/*.c)
OBJECTS = $(patsubst %.c, %.o, $(MODULES))
TEST_MODUlES = test_modules

ifeq ($(OS), Linux)
	LDLIBS = -lcheck_pic -pthread -lrt -lm -lsubunit
else
	LDLIBS = -lcheck -lm
endif

all: clean install docker

docker:
	docker-compose up --build

install:
	@echo "Start installation"
	@mkdir -p build/
	@cd sudoku && qmake && make && make clean
	mv sudoku/sudoku build/.
	# @cp -R ../build/$(TARGET).app ~/Desktop/$(TARGET).app
	@echo "┏=========================================┓"
	@echo "┃    The installation was successful.     ┃"
	@echo "┃     The project is located in the       ┃"
	@echo "┃    build folder and on the desktop.     ┃"
	@echo "┗=========================================┛"

uninstall:
	@rm -rf build
	@echo "Uninstall completed successfully"

style_check:
	@echo "┏=========================================┓"
	@echo "┃  Checking your code for Google Style    ┃"
	@echo "┗=========================================┛"
	@cp ../materials/linters/.clang-format .
	@clang-format -n **/*.c **/*.h view/3dViewerUI/*.h view/3dViewerUI/*.cpp
	@rm ./.clang-format

format_style:
	@echo "┏=========================================┓"
	@echo "┃  Formatting your code for Google Style  ┃"
	@echo "┗=========================================┛"
	@cp ../materials/linters/.clang-format .
	@clang-format -i **/*.c **/*.h view/3dViewerUI/*.h view/3dViewerUI/*.cpp
	@rm ./.clang-format

valgrind_check: test
	@CK_FORK=no valgrind --vgdb=no --leak-check=full \
	--show-leak-kinds=all --track-origins=yes --log-file="unit_tests_vg.log" -v --verbose -q --quiet -s tests/$(TEST_MODUlES)
	@echo "┏=========================================┓"
	@echo "┃        Valgrind check is over           ┃"
	@echo "┃       logs have been generated          ┃"
	@echo "┗=========================================┛"

clean:
	@rm -rf tests/logs/*.log tests/$(TEST_MODUlES) obj *.dSYM **/*.dSYM report test_coverage.info

fclean: clean
	@rm -rf ../build ../dist ~/Desktop/$(TARGET).app unit_tests_vg.log

.PHONY: all install uninstall dvi dist test gcov_report valgrind_check style_check format_style fonts clean fclean
