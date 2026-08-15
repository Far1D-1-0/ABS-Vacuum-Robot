add_executable(aspiradora)

target_sources(aspiradora
	PRIVATE
		main.cpp
)

target_link_libraries(aspiradora
	PRIVATE
		SFML::System
		SFML::Graphics
		SFML::Window
		SFML::Network
		SFML::Audio
		Libs
	PUBLIC
		cppyabm::cppyabm
)
