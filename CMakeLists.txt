cmake_minimum_required(VERSION 3.20)
project(Graphics)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_library(GLUT_LIBRARY GLUT)
include_directories(${OPENGL_INCLUDE_DIRS})

add_executable(EllipseAlgorithms EllipseAlgorithms.cpp
        EllipseAlgorithms.cpp
        EllipseAlgorithms.cpp
        EllipseAlgorithms.cpp
        EllipseAlgorithms.cpp)
target_link_libraries(EllipseAlgorithms ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
target_compile_options(EllipseAlgorithms PRIVATE -Wno-deprecated)

add_executable(Curves Curves.cpp
        Curves.cpp
)
target_link_libraries(Curves ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
target_compile_options(Curves PRIVATE -Wno-deprecated)

add_executable(Faces Faces.cpp
        Faces.cpp
)
target_link_libraries(Faces ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
target_compile_options(Faces PRIVATE -Wno-deprecated)