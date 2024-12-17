#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QString>
#include <pqxx/pqxx>
#include <vector>
#include <tuple>
#include <iostream>
#include <sstream>

#define SIZE 9
#define TABLE_SIZE (SIZE * SIZE)

// Определите вашу логику из предыдущего кода здесь...

// Основной класс приложения
class SudokuSolverApp : public QMainWindow {
    Q_OBJECT

public:
    SudokuSolverApp(QWidget *parent = nullptr);
    ~SudokuSolverApp();

private slots:
    void onLoadTaskButtonClicked();
    void onSolveButtonClicked();
    
private:
    void updateTable(const std::vector<std::vector<int>>& grid);
    void solveAndDisplay();
    
    QTableWidget *tableWidget;
    QPushButton *loadTaskButton;
    QPushButton *solveButton;
    pqxx::connection conn;
    int currentTaskId;
    std::vector<std::vector<int>> currentGrid;
    std::vector<int> topSums, bottomSums, leftSums, rightSums;
};

// Конструктор
SudokuSolverApp::SudokuSolverApp(QWidget *parent)
    : QMainWindow(parent), conn("dbname=your_db_name user=your_user password=your_password") {
    setWindowTitle("Sudoku Solver");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    tableWidget = new QTableWidget(SIZE, SIZE);
    layout->addWidget(tableWidget);

    loadTaskButton = new QPushButton("Load Sudoku Task");
    layout->addWidget(loadTaskButton);
    
    solveButton = new QPushButton("Solve Sudoku");
    layout->addWidget(solveButton);
    
    connect(loadTaskButton, &QPushButton::clicked, this, &SudokuSolverApp::onLoadTaskButtonClicked);
    connect(solveButton, &QPushButton::clicked, this, &SudokuSolverApp::onSolveButtonClicked);
}

// Деструктор
SudokuSolverApp::~SudokuSolverApp() {
}

// Метод для загрузки задачи из базы данных
void SudokuSolverApp::onLoadTaskButtonClicked() {
    try {
        auto [task_id, grid, top_sums, bottom_sums, left_sums, right_sums] = getSudokuTaskFromDB(conn);
        currentTaskId = task_id;
        currentGrid = grid;
        this->topSums = top_sums;
        this->bottomSums = bottom_sums;
        this->leftSums = left_sums;
        this->rightSums = right_sums;

        updateTable(currentGrid);
    } catch (const std::exception &e) {
        // Обработайте исключения (например, если задачи нет)
        std::cout << e.what() << std::endl;
    }
}

// Метод для отображения текущей задачи в таблице
void SudokuSolverApp::updateTable(const std::vector<std::vector<int>>& grid) {
    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            tableWidget->setItem(row, col, new QTableWidgetItem(QString::number(grid[row][col])));
        }
    }
}

// Метод для решения задачи судоку
void SudokuSolverApp::onSolveButtonClicked() {
    if (solveSudoku(currentGrid, topSums, bottomSums, leftSums, rightSums)) {
        updateTable(currentGrid);
        updateSudokuTaskStatus(conn, currentTaskId, "solved", serializeGrid(currentGrid));
    } else {
        std::cout << "No solution found!" << std::endl;
    }
}

// Метод для сериализации решения в строку
std::string SudokuSolverApp::serializeGrid(const std::vector<std::vector<int>>& grid) {
    std::string result = "";
    for (const auto& row : grid) {
        for (int num : row) {
            result += std::to_string(num) + " ";
        }
    }
    return result;
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    SudokuSolverApp w;
    w.show();
    return a.exec();
}
