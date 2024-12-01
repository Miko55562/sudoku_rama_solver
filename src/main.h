#include <iostream>
#include <vector>
#include <pqxx/pqxx>  
#include <sstream>

const int SIZE = 9;

bool isValid(const std::vector<std::vector<int>>& grid, int row, int col, int num,
             const std::vector<int>& topSums, const std::vector<int>& bottomSums,
             const std::vector<int>& leftSums, const std::vector<int>& rightSums);

bool solveSudoku(std::vector<std::vector<int>>& grid,
                 const std::vector<int>& topSums, const std::vector<int>& bottomSums,
                 const std::vector<int>& leftSums, const std::vector<int>& rightSums);

void printSudoku(const std::vector<std::vector<int>>& grid,
                 const std::vector<int>& topSums, const std::vector<int>& bottomSums,
                 const std::vector<int>& leftSums, const std::vector<int>& rightSums);

