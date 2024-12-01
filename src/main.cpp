#include <iostream>
#include <vector>
#include <tuple>
#include <sstream>
#include <pqxx/pqxx>

const int SIZE = 9;  // Размер судоку

// Функция для преобразования строки с числами в вектор
template <typename T>
std::vector<T> parseCSV(const std::string& str) {
    std::vector<T> result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        result.push_back(std::stoi(item));  // Для числовых данных
    }
    return result;
}

// Проверка на допустимость значения в ячейке
bool isValid(const std::vector<std::vector<int>>& grid, int row, int col, int num,
             const std::vector<int>& topSums, const std::vector<int>& bottomSums,
             const std::vector<int>& leftSums, const std::vector<int>& rightSums) {
    for (int i = 0; i < SIZE; ++i) {
        if (grid[row][i] == num || grid[i][col] == num) return false;
    }

    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (grid[startRow + i][startCol + j] == num) return false;
        }
    }

    return true;
}

// Решение судоку методом backtracking
bool solveSudoku(std::vector<std::vector<int>>& grid,
                 const std::vector<int>& topSums, const std::vector<int>& bottomSums,
                 const std::vector<int>& leftSums, const std::vector<int>& rightSums) {
    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            if (grid[row][col] == 0) {
                for (int num = 1; num <= 9; ++num) {
                    if (isValid(grid, row, col, num, topSums, bottomSums, leftSums, rightSums)) {
                        grid[row][col] = num;
                        if (solveSudoku(grid, topSums, bottomSums, leftSums, rightSums)) return true;
                        grid[row][col] = 0;
                    }
                }
                return false;
            }
        }
    }
    return true;
}

// Печать судоку
void printSudoku(const std::vector<std::vector<int>>& grid) {
    for (const auto& row : grid) {
        for (int num : row) {
            std::cout << num << " ";
        }
        std::cout << std::endl;
    }
}

// Извлечение задачи судоку из базы данных
std::tuple<std::vector<std::vector<int>>, std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>> getSudokuTaskFromDB(pqxx::connection& conn) {
    pqxx::work txn(conn);
    pqxx::result res = txn.exec("SELECT id, puzzle_grid, top_sums, bottom_sums, left_sums, right_sums FROM sudoku_puzzles WHERE status = 'pending' LIMIT 1");

    if (res.empty()) {
        throw std::runtime_error("No pending Sudoku tasks found.");
    }

    int task_id = res[0][0].as<int>();
    std::string puzzle = res[0][1].as<std::string>();
    std::vector<int> topSums = parseCSV<int>(res[0][2].as<std::string>());
    std::vector<int> bottomSums = parseCSV<int>(res[0][3].as<std::string>());
    std::vector<int> leftSums = parseCSV<int>(res[0][4].as<std::string>());
    std::vector<int> rightSums = parseCSV<int>(res[0][5].as<std::string>());

    // Преобразование строки в матрицу 9x9
    std::vector<std::vector<int>> grid(SIZE, std::vector<int>(SIZE, 0));
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            grid[i][j] = puzzle[i * SIZE + j] - '0';
        }
    }

    return {grid, topSums, bottomSums, leftSums, rightSums};
}

// Обновление статуса задачи судоку
void updateSudokuTaskStatus(pqxx::connection& conn, int task_id, const std::string& status, const std::string& solution = "") {
    pqxx::work txn(conn);
    std::string query = "UPDATE sudoku_puzzles SET status = " + txn.quote(status) + ", puzzle_grid = " + txn.quote(solution) +
                        " WHERE id = " + std::to_string(task_id);
    txn.exec(query);
    txn.commit();
}

int main() {
    try {
        pqxx::connection conn("dbname=postgres user=hello_django password=hello_django host=pgdb port=5432");

        // Получаем задачу
        auto [sudokuGrid, topSums, bottomSums, leftSums, rightSums] = getSudokuTaskFromDB(conn);

        // Решаем судоку
        if (solveSudoku(sudokuGrid, topSums, bottomSums, leftSums, rightSums)) {
            printSudoku(sudokuGrid);

            std::string solution;
            for (const auto& row : sudokuGrid) {
                for (int num : row) {
                    solution += std::to_string(num);
                }
            }
            std::cout << "solved";
            updateSudokuTaskStatus(conn, 1, "solved", solution);
        } else {
            std::cout << "No solution exists!" << std::endl;
            std::cout << "failed";
            updateSudokuTaskStatus(conn, 1, "failed");
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
