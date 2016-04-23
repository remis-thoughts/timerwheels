cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR)
project (timerwheels VERSION 1.0.0)

# The Library Itself
add_library(timerwheels INTERFACE)
target_include_directories(timerwheels INTERFACE "${PROJECT_SOURCE_DIR}/include")
install(FILES ${PROJECT_SOURCE_DIR}/include/timerwheels.h DESTINATION include)

# Unit tests
add_subdirectory(test)

# pkg-config
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
  @ONLY
)
install(FILES ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
