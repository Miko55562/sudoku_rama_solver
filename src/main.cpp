#include <iostream>
#include <vector>
#include <tuple>
#include <sstream>
#include <pqxx/pqxx>
#include <crow.h>

const int SIZE = 9;  // Размер судоку
std::vector<std::vector<int>> TABLE;

std::string cleanInput(const std::string& str) {
    std::string result = str;

    // Удаляем все '{' и '}'
    result.erase(std::remove(result.begin(), result.end(), '{'), result.end());
    result.erase(std::remove(result.begin(), result.end(), '}'), result.end());

    // Удаляем лишние пробелы вокруг
    result.erase(0, result.find_first_not_of(" \t"));
    result.erase(result.find_last_not_of(" \t") + 1);

    return result;
}

// Функция для преобразования строки с числами в вектор
template <typename T>
std::vector<T> parseCSV(const std::string& str) {
    std::vector<T> result;

    // Создаем копию строки с очищенными символами
    std::string cleanedStr = cleanInput(str);

    std::stringstream ss(cleanedStr);
    std::string item;

    while (std::getline(ss, item, ',')) {
        try {
            // Удаляем пробелы у текущего элемента
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t") + 1);

            if constexpr (std::is_same<T, int>::value) {
                result.push_back(std::stoi(item));  // Для чисел
            } else if constexpr (std::is_same<T, std::string>::value) {
                result.push_back(item);  // Для строк
            } else {
                throw std::invalid_argument("Unsupported type for parseCSV");
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing item: '" << item << "'. Reason: " << e.what() << std::endl;
        }
    }

    return result;
}



// Проверка на допустимость значения в ячейке
bool isValid(const std::vector<std::vector<int>>& grid, int row, int col, int num,
             const std::vector<int>& top_sums, const std::vector<int>& bottom_sums,
             const std::vector<int>& left_sums, const std::vector<int>& right_sums) {
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
                 const std::vector<int>& top_sums, const std::vector<int>& bottom_sums,
                 const std::vector<int>& left_sums, const std::vector<int>& right_sums) {
    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            TABLE[row][col] = 0;
        }
    }
    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            if (grid[row][col] == 0) {
                for (int num = 1; num <= 9; ++num) {
                    // std::cout << num;
                    if (isValid(grid, row, col, num, top_sums, bottom_sums, left_sums, right_sums)) {
                        grid[row][col] = num;
                        if (solveSudoku(grid, top_sums, bottom_sums, left_sums, right_sums)) return true;
                        grid[row][col] = 0;
                    }
                    TABLE[row][col] = grid[row][col];
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
std::tuple<int, std::vector<std::vector<int>>, std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>> getSudokuTaskFromDB(pqxx::connection& conn) {
    pqxx::work txn(conn);
    pqxx::result res = txn.exec("SELECT id, puzzle_grid, top_sums, bottom_sums, left_sums, right_sums FROM sudoku_puzzles WHERE status = 'unsolved' LIMIT 1");

    if (res.empty()) {
        throw std::runtime_error("No pending Sudoku tasks found.");
    }
    int task_id = res[0][0].as<int>();
    std::string puzzle = res[0][1].as<std::string>();
    std::vector<int> top_sums = parseCSV<int>(res[0][2].as<std::string>());
    std::vector<int> bottom_sums = parseCSV<int>(res[0][3].as<std::string>());
    std::vector<int> left_sums = parseCSV<int>(res[0][4].as<std::string>());
    std::vector<int> right_sums = parseCSV<int>(res[0][5].as<std::string>());

    // Преобразование строки в матрицу 9x9
    std::vector<std::vector<int>> grid(SIZE, std::vector<int>(SIZE, 0));
    
    std::istringstream iss(puzzle);
    // Убираем внешние квадратные скобки
    int num;
    int row, col = 0;
    while (iss >> num) {
        grid[row][col] = num;
        col++;
        if (col == SIZE) {  // Переходим на следующую строку
            col = 0;
            row++;
        }
    }

    return {task_id, grid, top_sums, bottom_sums, left_sums, right_sums};
}

// Обновление статуса задачи судоку
void updateSudokuTaskStatus(pqxx::connection& conn, int task_id, const std::string& status, const std::string& solution = "") {
    pqxx::work txn(conn);
    std::string query = "UPDATE sudoku_puzzles SET status = " + txn.quote(status) + ", puzzle_grid = " + txn.quote(solution) +
                        " WHERE id = " + std::to_string(task_id);
    txn.exec(query);
    txn.commit();
}

void createTable(pqxx::connection &conn) {
    std::string create_table_query = R"(
        CREATE TABLE IF NOT EXISTS sudoku_puzzles (
            id SERIAL PRIMARY KEY,
            puzzle_grid TEXT NOT NULL,
            top_sums INTEGER[] NOT NULL,
            bottom_sums INTEGER[] NOT NULL,
            left_sums INTEGER[] NOT NULL,
            right_sums INTEGER[] NOT NULL,
            status VARCHAR(50) DEFAULT 'unsolved'
        );
    )";

    pqxx::work txn(conn);
    txn.exec(create_table_query);
    txn.commit();
    std::cout << "Table 'sudoku_puzzles' created successfully (if not existed)." << std::endl;
}

void insertData(pqxx::connection &conn) {
    std::string insert_data_query = R"(
        INSERT INTO sudoku_puzzles (puzzle_grid, top_sums, bottom_sums, left_sums, right_sums, status)
        VALUES (
            '0 0 0 0 0 0 0 0 0 0 7 0 3 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0',
            '{14,15,16,13,20,12,13,13,19}',
            '{16,22,7,19,8,18,20,14,11}',
            '{14,16,15,12,14,19,16,20,9}',
            '{17,9,19,13,12,20,14,8,23}',
            'unsolved'
        );
    )";

    pqxx::work txn(conn);
    txn.exec(insert_data_query);
    txn.commit();
    std::cout << "Data inserted successfully." << std::endl;
}

std::string puzzleToString(const std::vector<std::vector<int>>& puzzle) {
    std::string result;
    for (const auto& row : puzzle) {
        for (int val : row) {
            result += std::to_string(val) + " ";
        }
        result += "\n"; // Add newline after each row
    }
    return result;
}

std::string generateJsonTable(const std::vector<std::vector<int>>& table) {
    std::ostringstream json;
    json << "[";

    for (size_t i = 0; i < table.size(); ++i) {
        json << "[";
        for (size_t j = 0; j < table[i].size(); ++j) {
            json << table[i][j];
            if (j < table[i].size() - 1) {
                json << ",";
            }
        }
        json << "]";
        if (i < table.size() - 1) {
            json << ",";
        }
    }

    json << "]";
    return json.str();
}


int main() {
    pqxx::connection conn("dbname=postgres user=hello_django password=hello_django host=pgdb port=5432");

    // createTable(conn);
    insertData(conn);

    crow::SimpleApp app;

    CROW_ROUTE(app, "/get_sudoku")
        .methods("GET"_method)
        ([]{
            auto page = generateJsonTable(TABLE);
            return page;
        });

    CROW_ROUTE(app, "/")([&conn](){
        auto page = crow::mustache::load_text("index.html");
        auto [id, puzzle_grid, top_sums, bottom_sums, left_sums, right_sums] = getSudokuTaskFromDB(conn);
        solveSudoku(puzzle_grid, top_sums, bottom_sums, left_sums, right_sums);
        return page;
    });

    app.loglevel(crow::LogLevel::DEBUG);
    app.port(8080).multithreaded().run();
}
