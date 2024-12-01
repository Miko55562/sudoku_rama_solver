#include "main.h"

// Проверка на допустимость значения в ячейке
bool isValid(const std::vector<std::vector<int>>& grid, int row, int col, int num,
             const std::vector<int>& topSums, const std::vector<int>& bottomSums,
             const std::vector<int>& leftSums, const std::vector<int>& rightSums) {
    // Проверка строки и столбца на уникальность
    for (int i = 0; i < SIZE; ++i) {
        if (grid[row][i] == num || grid[i][col] == num) return false;
    }

    // Проверка блока 3x3 на уникальность
    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (grid[startRow + i][startCol + j] == num) return false;
        }
    }

    // Проверка суммы первых и последних трех ячеек строки
    if (col < 3) {
        int currentRowLeftSum = 0;
        for (int i = 0; i < 3; ++i) {
            currentRowLeftSum += (i == col ? num : grid[row][i]);
        }
        if (currentRowLeftSum > leftSums[row]) return false;
        if (col == 2 && currentRowLeftSum != leftSums[row]) return false;
    }
    if (col >= SIZE - 3) {
        int currentRowRightSum = 0;
        for (int i = SIZE - 3; i < SIZE; ++i) {
            currentRowRightSum += (i == col ? num : grid[row][i]);
        }
        if (currentRowRightSum > rightSums[row]) return false;
        if (col == SIZE - 1 && currentRowRightSum != rightSums[row]) return false;
    }

    // Проверка суммы первых и последних трех ячеек столбца
    if (row < 3) {
        int currentColTopSum = 0;
        for (int i = 0; i < 3; ++i) {
            currentColTopSum += (i == row ? num : grid[i][col]);
        }
        if (currentColTopSum > topSums[col]) return false;
        if (row == 2 && currentColTopSum != topSums[col]) return false;
    }
    if (row >= SIZE - 3) {
        int currentColBottomSum = 0;
        for (int i = SIZE - 3; i < SIZE; ++i) {
            currentColBottomSum += (i == row ? num : grid[i][col]);
        }
        if (currentColBottomSum > bottomSums[col]) return false;
        if (row == SIZE - 1 && currentColBottomSum != bottomSums[col]) return false;
    }

    return true;
}

// Решение судоку методом backtracking
bool solveSudoku(std::vector<std::vector<int>>& grid,
                 const std::vector<int>& topSums, const std::vector<int>& bottomSums,
                 const std::vector<int>& leftSums, const std::vector<int>& rightSums) {
    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            if (grid[row][col] == 0) { // Если ячейка пустая
                for (int num = 1; num <= 9; ++num) {
                    if (isValid(grid, row, col, num, topSums, bottomSums, leftSums, rightSums)) {
                        grid[row][col] = num; // Пробуем поставить число
                        if (solveSudoku(grid, topSums, bottomSums, leftSums, rightSums)) return true;
                        grid[row][col] = 0; // Возврат к исходному значению при неудаче
                    }
                }
                return false; // Если ни одно число не подошло
            }
        }
    }
    return true; // Судоку решено
}

void printSudoku(const std::vector<std::vector<int>>& grid,
                 const std::vector<int>& topSums, const std::vector<int>& bottomSums,
                 const std::vector<int>& leftSums, const std::vector<int>& rightSums) {
    // Печать верхних сумм
    std::cout << "    ";
    for (int j = 0; j < SIZE; ++j) {
        std::cout << topSums[j] << " ";
        if ((j + 1) % 3 == 0 && j != SIZE - 1) std::cout << "  ";
    }
    std::cout << "\n\n";

    // Печать основной сетки с левыми и правыми суммами
    for (int i = 0; i < SIZE; ++i) {
        std::cout << leftSums[i] << "   ";  // Левая сумма строки
        for (int j = 0; j < SIZE; ++j) {
            std::cout << grid[i][j] << " ";
            if ((j + 1) % 3 == 0 && j != SIZE - 1) std::cout << "| ";
        }
        std::cout << "  " << rightSums[i];  // Правая сумма строки
        std::cout << std::endl;
        
        // Разделитель между блоками 3x3
        if ((i + 1) % 3 == 0 && i != SIZE - 1) {
            std::cout << "    ---------------------" << std::endl;
        }
    }

    // Печать нижних сумм
    std::cout << "\n    ";
    for (int j = 0; j < SIZE; ++j) {
        std::cout << bottomSums[j] << " ";
        if ((j + 1) % 3 == 0 && j != SIZE - 1) std::cout << "  ";
    }
    std::cout << std::endl;
}


// Функция для преобразования строки в вектор
template <typename T>
std::vector<T> parseCSV(const std::string& str) {
    std::vector<T> result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        result.push_back(std::stoi(item)); // Для числовых данных
    }
    return result;
}

// Функция для извлечения задачи из базы данных
std::tuple<std::vector<std::vector<int>>, std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>> getSudokuTaskFromDB(pqxx::connection& conn) {
    pqxx::work txn(conn);
    pqxx::result res = txn.exec("SELECT puzzle, top_sums, bottom_sums, left_sums, right_sums FROM sudoku_tasks WHERE status = 'pending' LIMIT 1");

    if (res.empty()) {
        throw std::runtime_error("No pending Sudoku tasks found.");
    }

    std::string puzzle = res[0][0].as<std::string>();
    std::string top_sums = res[0][1].as<std::string>();
    std::string bottom_sums = res[0][2].as<std::string>();
    std::string left_sums = res[0][3].as<std::string>();
    std::string right_sums = res[0][4].as<std::string>();

    // Преобразование строк в векторы
    std::vector<std::vector<int>> grid(SIZE, std::vector<int>(SIZE, 0));
    int index = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            grid[i][j] = puzzle[index++] - '0';
        }
    }

    // Преобразование сумм
    std::vector<int> topSums = parseCSV<int>(top_sums);
    std::vector<int> bottomSums = parseCSV<int>(bottom_sums);
    std::vector<int> leftSums = parseCSV<int>(left_sums);
    std::vector<int> rightSums = parseCSV<int>(right_sums);

    return {grid, topSums, bottomSums, leftSums, rightSums};
}

// Функция для обновления статуса задачи в базе данных
void updateSudokuTaskStatus(pqxx::connection& conn, int task_id, const std::string& status, const std::string& solution = "") {
    pqxx::work txn(conn);
    std::string query = "UPDATE sudoku_tasks SET status = '" + txn.quote(status) + "', solution = " + txn.quote(solution) +
                        " WHERE id = " + std::to_string(task_id);
    txn.exec(query);
    txn.commit();
}

int main() {
    try {
        // Подключение к базе данных PostgreSQL
        pqxx::connection conn("dbname=sudoku user=postgres password=yourpassword host=localhost port=5432");

        // Получаем задачу из базы данных
        auto [sudokuGrid, topSums, bottomSums, leftSums, rightSums] = getSudokuTaskFromDB(conn);

        // Решаем судоку
        if (solveSudoku(sudokuGrid, topSums, bottomSums, leftSums, rightSums)) {
            printSudoku(sudokuGrid, topSums, bottomSums, leftSums, rightSums);
            
            // Преобразуем решение в строку
            std::string solution = "";
            for (const auto& row : sudokuGrid) {
                for (int num : row) {
                    solution += std::to_string(num);
                }
            }

            // Обновляем статус задачи в базе данных
            updateSudokuTaskStatus(conn, 1, "solved", solution);  // Пример с id = 1
        } else {
            std::cout << "No solution exists!" << std::endl;
            updateSudokuTaskStatus(conn, 1, "failed");  // Пример с id = 1
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}