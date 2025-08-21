cmake_minimum_required(VERSION 3.10)
project(example)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find all packages using pkg-config (including X11)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
pkg_check_modules(GDK REQUIRED gdk-3.0)
pkg_check_modules(X11 REQUIRED x11)

# Add your executable
add_executable(${PROJECT_NAME} webkit_rp2.c)

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${GTK3_INCLUDE_DIRS}
    ${WEBKIT2GTK_INCLUDE_DIRS}
    ${GDK_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${GTK3_LIBRARIES}
    ${WEBKIT2GTK_LIBRARIES}
    ${GDK_LIBRARIES}
    ${X11_LIBRARIES}
)

# Add compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE ${GTK3_CFLAGS_OTHER})
