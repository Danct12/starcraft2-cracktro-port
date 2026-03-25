cmake_minimum_required (VERSION 3.10)

get_filename_component(TARGET_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

set (PROJECT_NAME "sc2_cracktro")

project(${PROJECT_NAME})

# Include sub-projects.
add_subdirectory("external")

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)

set(SRCS
  glrez.cpp
  timer.cpp
)

# Add source to this project's executable.
add_executable(sc2_cracktro ${SRCS})
target_compile_options(sc2_cracktro PRIVATE "-funsigned-char")
target_link_libraries(sc2_cracktro PUBLIC glfw minifmod OpenGL::GL OpenGL::GLU)
