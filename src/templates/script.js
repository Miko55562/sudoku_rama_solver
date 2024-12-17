// Подключаемся к серверу WebSocket
const socket = new WebSocket('ws://localhost:8080/ws');

// Функция для обработки успешного подключения
socket.onopen = function(event) {
    console.log("Connected to WebSocket server.");
};

// Функция для обработки получения сообщения от сервера
socket.onmessage = function(event) {
    const puzzleGrid = event.data;
    console.log("Received puzzle state: ", puzzleGrid);
    
    // Обновляем отображение судоку на странице (например, выводим полученные данные)
    updateSudokuDisplay(puzzleGrid);
};

// Функция для обработки закрытия соединения
socket.onclose = function(event) {
    console.log("Disconnected from WebSocket server.");
};

// Функция для обработки ошибок
socket.onerror = function(error) {
    console.error("WebSocket error: ", error);
};

// Функция для обновления отображения судоку на странице
function updateSudokuDisplay(puzzleGrid) {
    const grid = puzzleGrid.split("\n").map(row => row.trim().split(" ").map(Number));
    // Здесь можно обновить DOM, чтобы отобразить состояние судоку
    const table = document.getElementById("sudoku-table");
    
    for (let i = 0; i < grid.length; i++) {
        for (let j = 0; j < grid[i].length; j++) {
            const cell = table.rows[i].cells[j];
            cell.textContent = grid[i][j] === 0 ? "" : grid[i][j];
        }
    }
}

// Функция для начала решения судоку
function startSudoku() {
    fetch('/start_sudoku', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
            console.log("Started solving sudoku: ", data);
        })
        .catch(error => {
            console.error("Error starting sudoku:", error);
        });
}
