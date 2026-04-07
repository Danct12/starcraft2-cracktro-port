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

set(RESRCS
  db_wot61.xm
  font.bmp
  logo.bmp
  razor.bmp
  scanline.bmp
)

add_executable(bin2h bin2h.c)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/resource")
foreach(_resrc ${RESRCS})
  string(REPLACE "." "_" _iresrc ${_resrc})
  add_custom_command(
    OUTPUT ${_iresrc}.h
    COMMAND bin2h -i "${CMAKE_CURRENT_SOURCE_DIR}/resource/${_resrc}"
        -o "${CMAKE_CURRENT_BINARY_DIR}/resource/${_iresrc}.h" -n "${_iresrc}" -s
    DEPENDS bin2h "${CMAKE_CURRENT_SOURCE_DIR}/resource/${_resrc}"
  )
  list(APPEND gen_resrc ${_iresrc}.h)
endforeach()
add_custom_target(do_gen_resrc DEPENDS ${gen_resrc})

# Add source to this project's executable.
add_executable(${PROJECT_NAME} ${SRCS})
add_dependencies(${PROJECT_NAME} do_gen_resrc)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/resource")
target_compile_options(${PROJECT_NAME} PRIVATE "-funsigned-char")
target_link_libraries(${PROJECT_NAME} PUBLIC glfw minifmod OpenGL::GL OpenGL::GLU)
