cmake_minimum_required(VERSION 3.16)
project(BigCalc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(BigCalc
    main.cpp
    crypto/BitcoinCrypto.cpp
    crypto/SecpAffine.cpp
)

target_include_directories(BigCalc PRIVATE .)
target_link_libraries(BigCalc PRIVATE Qt6::Widgets)

add_custom_command(TARGET BigCalc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/fonts $<TARGET_FILE_DIR:BigCalc>/fonts
)
