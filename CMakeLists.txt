cmake_minimum_required(VERSION 4.1)

project(aspiradora)

set(SFML_STATIC_LIBRARIES ON)
find_package(SFML 3.1 COMPONENTS Graphics System Window Network Audio REQUIRED)

find_package(cppyabm CONFIG REQUIRED)

set(ASPIRADORA_HEADERS_DIR
	${CMAKE_SOURCE_DIR}/include
)

add_subdirectory(src)
