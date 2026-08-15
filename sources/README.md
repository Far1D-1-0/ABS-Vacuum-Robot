commands for building and installing SFML

cmake --preset dev
cmake --build build --config Release
cmake --build build --config Debug

cmake --preset dev-static
cmake --build build --config Release
cmake --build build --config Debug

cmake --install build --config Debug
cmake --install build --config Release
