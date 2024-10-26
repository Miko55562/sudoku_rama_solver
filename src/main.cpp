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


int main() {
    // Пример решетки судоку
    std::vector<std::vector<int>> sudokuGrid = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 7, 0, 3, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    // Боковые суммы для первых и последних трех ячеек
    std::vector<int> topSums = {14, 15, 16, 13, 20, 12, 13, 13, 19};    // Верхние суммы столбцов
    std::vector<int> bottomSums = {16, 22, 7, 19, 8, 18, 20, 14, 11}; // Нижние суммы столбцов
    std::vector<int> leftSums = {14, 16, 15, 12, 14, 19, 16, 20, 9};   // Левые суммы строк
    std::vector<int> rightSums = {17, 9, 19, 13, 12, 20, 14, 8, 23};  // Правые суммы строк

    if (solveSudoku(sudokuGrid, topSums, bottomSums, leftSums, rightSums)) {
        printSudoku(sudokuGrid, topSums, bottomSums, leftSums, rightSums );
    } else {
        std::cout << "No solution exists!" << std::endl;
    }

    return 0;
}
