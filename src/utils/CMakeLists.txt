project(libutils)
file(GLOB SOURCES "*.cpp")
add_library(libutils SHARED ${SOURCES})

# link to other libraries
target_link_libraries(libutils ${OpenCV_LIBS} Eigen3::Eigen)

target_include_directories(libutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
