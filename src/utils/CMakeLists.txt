project(libutils)
file(GLOB SOURCES "*.cpp")
add_library(libutils ${SOURCES})

# link to other libraries
target_link_libraries(libutils)

target_include_directories(libutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
