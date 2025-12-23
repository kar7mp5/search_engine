cmake_minimum_required(VERSION 3.10)
project(MiniSearchEngine LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O2)
endif()

set(SOURCES
    main.c
    src/crawler.c
)

include_directories(src include)

add_executable(main ${SOURCES})

find_package(PkgConfig REQUIRED)

pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(GUMBO REQUIRED gumbo)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

target_include_directories(main PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${GUMBO_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
)

target_link_libraries(main PRIVATE
    ${CURL_LIBRARIES}
    ${GUMBO_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    pthread
)