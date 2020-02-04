cmake_minimum_required(VERSION 3.1)
project(quickconv C)

find_package(FFTW3f CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)

add_executable(quickconv
main.c
Timeline.c
box.c
readnumerictable.c
blur_plan.c
staticHelpers.c
)


target_link_libraries(quickconv PRIVATE
FFTW3::fftw3f
sqlite3
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/forConv.db 
${CMAKE_CURRENT_BINARY_DIR}/forConv.db COPYONLY)
