#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <cmath>
#include <random>
#include <chrono>
#include <Simulation/Simulation.h>

int main(){

	// Aux values
	constexpr float scale = 100.f;
	const sf::Font font("assets/arial.ttf");

	sf::RenderWindow window(sf::VideoMode({800, 800}), "Robot Aspiradora MAS", sf::Style::Default);
	
	// Load Grid
	const sf::Texture texture("assets/grid.png");
	sf::Sprite sprite(texture);
	sprite.setScale(sf::Vector2f(10.0f, 10.0f));

	// Array of tiles graphics
	vector<std::pair<sf::RectangleShape, std::shared_ptr<Tile>>> tiles;
	tiles.reserve(64);

	// Array of Aspiradoras graphics
	vector<std::pair<sf::CircleShape, std::shared_ptr<Aspiradora>>> agents;
	agents.reserve(10);

	// Timer
	auto last_time = std::chrono::steady_clock::now();
	std::chrono::milliseconds interval(300);

	// Initialize simulation
	auto envObj = make_shared<Room>(10);

	envObj->setup();
	
	// Initialize tiles graphics
	for (auto& [index, patch] : envObj->patches) {
		sf::RectangleShape shape;
	
		shape.setSize(sf::Vector2f(0.8, 0.8));

		if (patch->type == Tile::Type::TILE) {
			shape.setFillColor(sf::Color::White);
		} else if (patch->type == Tile::Type::CHRG_TILE) {
			shape.setFillColor(sf::Color::Yellow);
		}
		
		shape.setOrigin(shape.getSize() / 2.0f);
		shape.setPosition(sf::Vector2f(patch->coords[0], patch->coords[1]) * scale);
		shape.setScale(sf::Vector2f(scale, scale));

		tiles.emplace_back(std::move(shape), patch);

	}

	// Initialize aspiradoras graphics
	for (auto& agent : envObj->agents) {
		sf::CircleShape shape;

		shape.setRadius(0.4);
		shape.setFillColor(sf::Color::Blue);
		shape.setOrigin({shape.getRadius(), shape.getRadius()});
		shape.setPosition(sf::Vector2f(agent->_patch->coords[0], agent->_patch->coords[1]) * scale);
		shape.setScale({scale, scale});

		agents.emplace_back(std::move(shape), agent);
	}

    	// Start the game loop
    	while (window.isOpen())
    	{
		auto now = std::chrono::steady_clock::now();
		
		// Create a new step each interval
		if (now - last_time >= interval) {
			last_time = now;
			envObj->step();

			// Graph Tiles and agents in their updated state
			for (auto& [sh, pt] : tiles) {
				if (pt->dirty == true) {
					sh.setFillColor(sf::Color(100, 20, 20, 255));
				} else if (pt->type == Tile::Type::TILE) {
					sh.setFillColor(sf::Color::White);
				}
			}

			for (auto& [sh, ag] : agents) {
				sh.setPosition(sf::Vector2f(ag->_patch->coords[0], ag->_patch->coords[1]) * scale);
			}
		}

		// Process events
        	while (const std::optional event = window.pollEvent())
        	{
			// Close window: exit
            		if (event->is<sf::Event::Closed>())
	                window.close();
        	}

        	// Clear screen
        	window.clear();

	        // Draw the sprite
        	window.draw(sprite);

		// Draw tiles
		for (auto& [sh, pt] : tiles) {
			window.draw(sh);
		}

		// Draw aspiradoras
		for (auto& [sh, ag] : agents) {
			window.draw(sh);
			sf::Text text(font, ag->id);
			text.setCharacterSize(12);
			text.setStyle(sf::Text::Bold);
			text.setFillColor(sf::Color::Red);
			text.setPosition(sh.getPosition());
			window.draw(text);
		}

	        // Update the window
        	window.display();
	}
	envObj->export_csv();
}


