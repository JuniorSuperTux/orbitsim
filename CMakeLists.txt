cmake_minimum_required(VERSION 3.20)

project(orbitsim LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Cinder)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src coresrc)
add_executable(${PROJECT_NAME} WIN32 ${coresrc})

set_target_properties(${PROJECT_NAME}
	PROPERTIES CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	# BUILD_TYPE Debug
	EXPORT_COMPILE_COMMANDS ON)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE cinder)
