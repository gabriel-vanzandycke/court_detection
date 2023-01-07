project(libcourtdetector)
file(GLOB SOURCES "*.cpp")
add_library(libcourtdetector SHARED ${SOURCES})

target_link_libraries(libcourtdetector libutils ${OpenCV_LIBS} Eigen3::Eigen)

target_include_directories(libcourtdetector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
