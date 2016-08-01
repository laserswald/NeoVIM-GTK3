cmake_minimum_required(VERSION 3.3)
set(PROJECT_LABEL "neovim-gtk3")

project(${PROJECT_LABEL} C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(VTE REQUIRED vte-2.91)
include_directories(${GTK3_INCLUDE_DIRS} ${VTE_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS} ${VTE_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER} ${VTE_CFLAGS_OTHER})

set(SOURCES
    src/main.c
)

add_executable(${PROJECT_LABEL} ${SOURCES})
target_link_libraries(${PROJECT_LABEL} ${GTK3_LIBRARIES} ${VTE_LIBRARIES})