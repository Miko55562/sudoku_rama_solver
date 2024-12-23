#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

static constexpr int SIZE = 9;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadSudoku();   // Загрузка судоку из БД
    void solveSudoku();  // Объявление слота
//    void on_solveButton_clicked();

protected:
    void paintEvent(QPaintEvent *event) override;  // Добавляем объявление paintEvent

private:
    Ui::MainWindow *ui;
    QTableWidget *sudokuTable; // Таблица для отображения судоку
    QPushButton *loadButton;
    QPushButton *solveButton;
    QLabel *statusLabel;
    QGraphicsScene *scene;         // Сцена для рисования линий

    QVector<QVector<int>> grid; // Сетка судоку
    pqxx::connection *dbConn;   // Соединение с БД
    void drawGrid();

    bool solveSudokuStep(QVector<QVector<int>>& grid,
                                      const std::vector<int>& top_sums,
                                      const std::vector<int>& bottom_sums,
                                      const std::vector<int>& left_sums,
                                      const std::vector<int>& right_sums);
    bool isValid(const QVector<QVector<int>>& grid, int row, int col, int num,
                  const std::vector<int>& top_sums, const std::vector<int>& bottom_sums,
                  const std::vector<int>& left_sums, const std::vector<int>& right_sums);
    std::tuple<int, std::vector<std::vector<int>>, std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>> getSudokuTaskFromDB(pqxx::connection& conn);
    void updateTable();         // Обновление интерфейса
//    bool solveSudoku(QVector<QVector<int>>& grid);
//    bool isValid(int row, int col, int num);
};

#endif // MAINWINDOW_H
