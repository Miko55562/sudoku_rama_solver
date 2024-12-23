#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <sstream>
#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <pqxx/pqxx>
#include <QVector>
#include <tuple>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Подключение к PostgreSQL
    try {
        dbConn = new pqxx::connection("dbname=postgres user=hello_django password=hello_django host=127.0.0.1");        if (!dbConn->is_open()) {
            throw std::runtime_error("Failed to open the database.");
        }
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Database Error", e.what());
    }

    // Инициализация элементов интерфейса
    sudokuTable = new QTableWidget(SIZE, SIZE, this);  // Use QTableWidget, not QTableView
    sudokuTable->setStyleSheet(
        "QTableWidget {"
        "   border: 1px solid #ccc;"
        "   background-color: #f4f4f4;"
        "   gridline-color: #bbb;"
        "   font-size: 14px;"
        "   font-family: Arial;"
        "}"
        "QTableWidget::item {"
        "   padding: 10px;"
        "   text-align: center;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #7ec8e3;"
        "}"
    );
    sudokuTable->horizontalHeader()->setVisible(false);  // Скрыть заголовки столбцов
    sudokuTable->verticalHeader()->setVisible(false);    // Скрыть заголовки строк
    sudokuTable->setShowGrid(true);
    sudokuTable->setGridStyle(Qt::DashLine);  // Легкая сетка
    for (int i = 0; i < SIZE; ++i) {
        sudokuTable->setColumnWidth(i, 50);  // Установить ширину столбцов
        sudokuTable->setRowHeight(i, 50);     // Установить высоту строк
    }
    sudokuTable->setFixedSize(SIZE * 60, SIZE * 60);  // Устанавливаем фиксированный размер таблицы

    loadButton = new QPushButton("Load Sudoku", this);
    solveButton = new QPushButton("Solve Sudoku", this);
    statusLabel = new QLabel("Status: Waiting...", this);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(sudokuTable);
    layout->addWidget(loadButton);
    layout->addWidget(solveButton);
    layout->addWidget(statusLabel);
    ui->centralwidget->setLayout(layout);

    // Связывание кнопок со слотами
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadSudoku);
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::solveSudoku);
}

bool MainWindow::isValid(const QVector<QVector<int>>& grid, int row, int col, int num,
              const std::vector<int>& top_sums, const std::vector<int>& bottom_sums,
              const std::vector<int>& left_sums, const std::vector<int>& right_sums) {

    // Проверка на строку и столбец
    for (int i = 0; i < SIZE; ++i) {
        if (grid[row][i] == num || grid[i][col] == num) {
            return false;
        }
    }

    // Проверка на 3x3 блок
    int start_row = row - row % 3;
    int start_col = col - col % 3;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (grid[start_row + i][start_col + j] == num) {
                return false;
            }
        }
    }

    // Проверка на левую сумму
    if (start_col == 0 && std::accumulate(grid[row].begin() + start_col, grid[row].begin() + start_col + 2, 0) + num > left_sums[row]) {
        return false;
    }

    // Проверка на правую сумму
    if (start_col == 3 && std::accumulate(grid[row].begin() + start_col, grid[row].begin() + start_col + 2, 0) + num > 45 - (left_sums[row] + right_sums[row])) {
        return false;
    }

    // Проверка на правую сумму
    if (start_col == 6 && std::accumulate(grid[row].begin() + start_col, grid[row].begin() + start_col + 2, 0) + num > right_sums[row]) {
        return false;
    }

    // Проверка на верхнюю сумму
    if (start_row == 0) {
        int sum_top = 0;
        for (int i = 0; i < 3; ++i) {
            sum_top += grid[i][col];
        }
        if (sum_top + num > top_sums[col]) {
            return false;
        }
    }

    // Проверка на среднюю сумму
    if (start_row == 3) {
        int sum_middle = 0;
        for (int i = 3; i < 6; ++i) {
            sum_middle += grid[i][col];
        }
        if (sum_middle + num > 45 - (top_sums[col] + bottom_sums[col])) {
            return false;
        }
    }

    // Проверка на нижнюю сумму
    if (start_row == 6) {
        int sum_bottom = 0;
        for (int i = 6; i < 9; ++i) {
            sum_bottom += grid[i][col];
        }
        if (sum_bottom + num > bottom_sums[col]) {
            return false;
        }
    }

    return true;
}

bool MainWindow::solveSudokuStep(QVector<QVector<int>>& grid,
                                  const std::vector<int>& top_sums,
                                  const std::vector<int>& bottom_sums,
                                  const std::vector<int>& left_sums,
                                  const std::vector<int>& right_sums) {


    for (int row = 0; row < SIZE; ++row) {
        QApplication::processEvents();
        for (int col = 0; col < SIZE; ++col) {

            if (grid[row][col] == 0) {  // Если ячейка пуста
                for (int num = 1; num <= 9; ++num) {
                    if (isValid(grid, row, col, num, top_sums, bottom_sums, left_sums, right_sums)) {
                        grid[row][col] = num;  // Присваиваем число
                        QTableWidgetItem *item = new QTableWidgetItem(QString::number(num));
                        sudokuTable->setItem(row+1, col+1, item);
                        item->setTextAlignment(Qt::AlignCenter);


                        // Обновляем таблицу
                        updateTable();

                        if (solveSudokuStep(grid, top_sums, bottom_sums, left_sums, right_sums))  // Рекурсивный вызов
                            return true;
                        grid[row][col] = 0;  // Если не получилось, сбрасываем
                    }
                }
                return false;  // Если ни одно число не подходит, возвращаем false
            }
        }
    }
    for (const auto& row : grid) {
        for (int num : row) {
            qDebug() << num << " ";
        }
        qDebug() << "\n";
    }
    return true;  // Если все клетки заполнены, возвращаем true
}

void MainWindow::updateTable() {
    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            int value = grid[row][col];
            QTableWidgetItem *item = new QTableWidgetItem(QString::number(value));
            sudokuTable->setItem(row+1, col+1, item);
            item->setTextAlignment(Qt::AlignCenter);
//            item->setBackground(QBrush(Qt::black));

//            QThread::msleep(200); // Добавляем задержку для наглядности

        }
    }
}

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
            qDebug() << "Error parsing item: '";
        }
    }

    return result;
}


std::tuple<int, std::vector<std::vector<int>>, std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>> MainWindow::getSudokuTaskFromDB(pqxx::connection& conn) {
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

    int row = 0, col = 0;
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

void MainWindow::solveSudoku() {
    // Для демонстрации, мы будем решать судоку пошагово
    try {
        auto [task_id, gridFromDB, top_sums, bottom_sums, left_sums, right_sums] = getSudokuTaskFromDB(*dbConn);

        // Сохраняем полученную сетку
        grid = QVector<QVector<int>>(SIZE, QVector<int>(SIZE));
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                grid[i][j] = gridFromDB[i][j];
            }
        }

        // Заполняем таблицу текущим состоянием судоку
        updateTable();

        // Рекурсивное решение с выводом каждого шага
        solveSudokuStep(grid, top_sums, bottom_sums, left_sums, right_sums);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Database Error", e.what());
    }
}





void MainWindow::paintEvent(QPaintEvent *event) {
    QMainWindow::paintEvent(event);
    //drawGrid();  // Вызовем рисование сетки, чтобы линии рисовались поверх таблицы
}

void MainWindow::drawGrid() {
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(2);

    // Рисуем линии для разделения секций 3x3
    int cellSize = 50;
    for (int i = 0; i <= SIZE; ++i) {
        if (i % 3 == 0) {
            pen.setWidth(3);  // Более толстые линии для секций 3x3
        } else {
            pen.setWidth(1);  // Тонкие линии для других
        }

        // Вертикальные линии
        scene->addLine(i * cellSize, 0, i * cellSize, SIZE * cellSize, pen);
        // Горизонтальные линии
        scene->addLine(0, i * cellSize, SIZE * cellSize, i * cellSize, pen);
    }
}

void MainWindow::loadSudoku() {
    try {
        auto [task_id, gridFromDB, top_sums, bottom_sums, left_sums, right_sums] = getSudokuTaskFromDB(*dbConn);

        // Сохраняем полученную сетку
        grid = QVector<QVector<int>>(SIZE, QVector<int>(SIZE));
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                grid[i][j] = gridFromDB[i][j];
            }
        }

        // Увеличиваем размер таблицы для отображения сумм
        sudokuTable->setRowCount(SIZE + 2);
        sudokuTable->setColumnCount(SIZE + 2);

        // Очищаем таблицу и обновляем её содержимое
        sudokuTable->clearSpans();
        for (int row = 0; row < SIZE; ++row) {
            for (int col = 0; col < SIZE; ++col) {
                int value = grid[row][col];
                QTableWidgetItem *item = new QTableWidgetItem(QString::number(value));
                sudokuTable->setItem(row + 1, col + 1, item);
                item->setTextAlignment(Qt::AlignCenter);

//                // Цвет для пустых ячеек
//                if (value == 0) {
//                    item->setTextColor(Qt::gray);
//                }

                // Добавляем черные линии для 3x3 секций
                if ((row % 3 == 0 && row != 0) || (col % 3 == 0 && col != 0)) {
//                    item->setBackground(QBrush(Qt::black));  // Применить черную заливку
                }
            }
        }

        // Заполнение сумм сверху и снизу
        for (int col = 0; col < SIZE; ++col) {
            QTableWidgetItem *topSumItem = new QTableWidgetItem(QString::number(top_sums[col]));
            topSumItem->setTextAlignment(Qt::AlignCenter);
            sudokuTable->setItem(0, col + 1, topSumItem);

            QTableWidgetItem *bottomSumItem = new QTableWidgetItem(QString::number(bottom_sums[col]));
            bottomSumItem->setTextAlignment(Qt::AlignCenter);
            sudokuTable->setItem(SIZE + 1, col + 1, bottomSumItem);
        }

        // Заполнение сумм слева и справа
        for (int row = 0; row < SIZE; ++row) {
            QTableWidgetItem *leftSumItem = new QTableWidgetItem(QString::number(left_sums[row]));
            leftSumItem->setTextAlignment(Qt::AlignCenter);
            sudokuTable->setItem(row + 1, 0, leftSumItem);

            QTableWidgetItem *rightSumItem = new QTableWidgetItem(QString::number(right_sums[row]));
            rightSumItem->setTextAlignment(Qt::AlignCenter);
            sudokuTable->setItem(row + 1, SIZE + 1, rightSumItem);
        }



        // Настройка таблицы, чтобы она выглядела как поле судоку
        sudokuTable->horizontalHeader()->setVisible(false);
        sudokuTable->verticalHeader()->setVisible(false);
        sudokuTable->setShowGrid(true);
        sudokuTable->setGridStyle(Qt::SolidLine);
        sudokuTable->setFixedSize((SIZE + 2) * 51, (SIZE + 2) * 51);
        for (int i = 0; i < SIZE + 2; ++i) {
            sudokuTable->setColumnWidth(i, 5);
            sudokuTable->setRowHeight(i, 50);
        }

        statusLabel->setText("Sudoku Loaded! Task ID: " + QString::number(task_id));
        qDebug() << "Task ID:" << task_id;
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Database Error", e.what());
    }
}





MainWindow::~MainWindow()
{
    delete ui;
}
