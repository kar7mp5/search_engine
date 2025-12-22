# CMakeLists.txt - 간단하고 안정적인 버전

cmake_minimum_required(VERSION 3.10)
project(MiniSearchEngine LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 기본 컴파일 옵션
add_compile_options(-Wall -Wextra)

# 빌드 타입에 따른 최적화
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O2)
endif()

# 소스 파일 목록 (파일 추가 시 여기만 수정)
set(SOURCES
    src/main.c
)

include_directories(src include)

add_executable(mini_search ${SOURCES})

# pkg-config 찾기
find_package(PkgConfig REQUIRED)

# 라이브러리 연결
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(GUMBO REQUIRED gumbo)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

target_include_directories(mini_search PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${GUMBO_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
)

target_link_libraries(mini_search PRIVATE
    ${CURL_LIBRARIES}
    ${GUMBO_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    pthread
)