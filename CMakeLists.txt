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

# Man pages, see: https://majewsky.wordpress.com/2010/08/14/tip-of-the-day-cmake-and-doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf.in ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf @ONLY)
	add_custom_target(
		doc ALL ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/man DESTINATION share USE_SOURCE_PERMISSIONS)
endif(DOXYGEN_FOUND)
