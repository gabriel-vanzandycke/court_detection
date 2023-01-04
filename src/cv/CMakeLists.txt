project(libcv)
file(GLOB SOURCES "*.cpp")
add_library(libcv ${SOURCES})

# link to other libraries
target_link_libraries(libcv libutils)

target_include_directories(libcv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
