add_library(Simulation INTERFACE)

target_sources(Simulation
	INTERFACE
		FILE_SET simulation
		TYPE HEADERS
		BASE_DIRS ${ASPIRADORA_HEADERS_DIR}
		FILES 
			${ASPIRADORA_HEADERS_DIR}/Simulation.h
)

target_link_libraries(Simulation
	INTERFACE
		cppyabm::cppyabm
)
