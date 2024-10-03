cmake_minimum_required(VERSION 3.16)
project(
    qt_snake_game
    VERSION 0.0.1
    DESCRIPTION "Simple snake game using qt"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

set(sources 
    src/main.cpp
    src/snake.cpp
)

# Найти компоненты библиотеки Qt5 или прервать компиляцию
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Добавляет target: тест библиотеки Qt5
add_executable(${PROJECT_NAME} ${sources})

# Указывает директории, в которых будет осуществляться поиск h-файлов
target_include_directories(${PROJECT_NAME}  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Указывает зависимость target'а от библиотеки Qt5
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets)