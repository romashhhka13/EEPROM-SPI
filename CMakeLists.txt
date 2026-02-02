cmake_minimum_required(VERSION 3.10)

project(spi_eeprom_driver
    VERSION 0.1
    DESCRIPTION "SPI EEPROM driver using bit-banging"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(demo
    src/main.cpp
    src/eeprom_25lc040a.cpp
)