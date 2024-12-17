document.addEventListener("DOMContentLoaded", function() {
    const solveButton = document.getElementById("solveButton");
    const sudokuTable = document.getElementById("sudoku-table");

    // Функция для генерации таблицы судоку
    function createSudokuTable(grid) {
        const table = document.createElement("table");

        for (let i = 0; i < 9; i++) {
            const row = document.createElement("tr");

            for (let j = 0; j < 9; j++) {
                const cell = document.createElement("td");
                const input = document.createElement("input");
                input.type = "text";
                input.maxLength = 1;
                input.value = grid[i][j] !== 0 ? grid[i][j] : ""; // Заполняем пустыми клетками
                cell.appendChild(input);
                row.appendChild(cell);
            }
            table.appendChild(row);
        }

        sudokuTable.innerHTML = "";
        sudokuTable.appendChild(table);
    }

    // Функция для получения задачи судоку с сервера
    function getSudokuTask() {
        fetch("http://localhost:8080")
            .then(response => response.json())
            .then(data => {
                const grid = data.puzzle_grid; // Здесь предполагаем, что ответ от сервера содержит сетку
                createSudokuTable(grid);
            })
            .catch(error => console.error("Error fetching Sudoku task:", error));
    }

    // Функция для решения судоку
    function solveSudoku() {
        const grid = [];

        // Собираем значения из таблицы
        const rows = sudokuTable.querySelectorAll("tr");
        rows.forEach((row, i) => {
            const cells = row.querySelectorAll("td input");
            const gridRow = [];
            cells.forEach((cell, j) => {
                gridRow.push(cell.value ? parseInt(cell.value) : 0);
            });
            grid.push(gridRow);
        });

        // Отправляем запрос на сервер для решения
        fetch("http://localhost:8080", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({ puzzle_grid: grid })
        })
        .then(response => response.json())
        .then(data => {
            const solvedGrid = data.solution; // Здесь предполагаем, что сервер возвращает решенную сетку
            createSudokuTable(solvedGrid);
        })
        .catch(error => console.error("Error solving Sudoku:", error));
    }

    // Запрос на получение задачи судоку при загрузке страницы
    getSudokuTask();

    // Обработчик клика по кнопке для решения судоку
    solveButton.addEventListener("click", solveSudoku);
});
