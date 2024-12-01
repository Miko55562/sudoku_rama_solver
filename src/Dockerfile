FROM ubuntu:latest

# RUN groupadd -r sample && useradd -r -g sample sample
# USER sample
# Установим необходимые пакеты
RUN apt-get update && apt-get install -y \
    build-essential \
    libpqxx-dev \
    postgresql-client \
    g++ \
    make \
    cmake

# Копируем исходный код и компилируем его
WORKDIR /app
COPY . ./
RUN g++ -o sudoku_solver main.cpp -lpqxx -lpq

# Запуск программы
CMD g++ -o sudoku_solver main.cpp -lpqxx -lpq && ./sudoku_solver

# ENTRYPOINT [ "./sudoku_solver" ]
