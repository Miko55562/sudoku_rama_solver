cmake_minimum_required(VERSION 3.8)

project(project)

# full path from root directory
include(/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(Crow CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)

include_directories(${LIBPQXX_INCLUDE_DIRS} ${PostgreSQL_INCLUDE_DIRS})
link_directories(${LIBPQXX_LIBRARY_DIRS} ${PostgreSQL_LIBRARY_DIRS})

add_executable(
    app
    main.cpp
)

target_link_libraries(app PUBLIC Crow::Crow)
target_link_libraries(app PRIVATE pqxx pq)