FROM ubuntu:latest

# RUN groupadd -r sample && useradd -r -g sample sample
# USER sample
# Установим необходимые пакеты
RUN sed -i 's|http://archive.ubuntu.com/ubuntu|http://mirror.yandex.ru/ubuntu|g' /etc/apt/sources.list && \
    apt-get update && apt-get install -y \
    git \
    build-essential \
    libpqxx-dev \
    postgresql-client \
    g++ \
    make \
    cmake

RUN apt-get install --reinstall ca-certificates -y

# install developer dependencies
RUN apt-get install -y git build-essential cmake --no-install-recommends

# install vcpkg package manager
RUN git clone https://github.com/microsoft/vcpkg
RUN apt-get install -y curl zip
RUN vcpkg/bootstrap-vcpkg.sh

# install crow package
RUN /vcpkg/vcpkg install crow

# Копируем исходный код и компилируем его
# WORKDIR /app
# COPY . ./

# RUN g++ -std=c++17 -o sudoku_solver main.cpp -lpqxx -lboost_system -lpqxx -pthread

# copy files from local directory to container
COPY . /project

# define working directory from container
WORKDIR /build

# compile with CMake 
RUN bash -c "cmake ../project && cmake --build . && cp -r ../project/templates ."

EXPOSE 8080

# Запуск программы
CMD ["./app"]


# ENTRYPOINT [ "./sudoku_solver" ]
