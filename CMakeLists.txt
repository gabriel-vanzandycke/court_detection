cmake_minimum_required(VERSION 3.25.1)
project(court_detection VERSION 0.0.1)
set(CMAKE_BUILD_TYPE Debug)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)
find_package(Eigen3 3.3 REQUIRED)

add_subdirectory(src)

add_executable(app.exe main.cpp)
target_link_libraries(app.exe PRIVATE libcourtdetector libutils ${OpenCV_LIBS})
