project(libcourt_detection)
file(GLOB SOURCES "*.cpp")
add_library(libcourt_detection SHARED ${SOURCES})

target_link_libraries(libcourt_detection libutils ${OpenCV_LIBS} Eigen3::Eigen)

target_include_directories(libcourt_detection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
