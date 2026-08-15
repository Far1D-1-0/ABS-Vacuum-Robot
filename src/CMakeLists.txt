add_subdirectory(Simulation)

add_library(Libs INTERFACE)

target_link_libraries(Libs
	INTERFACE
		Simulation
)

add_subdirectory(aspiradora)
