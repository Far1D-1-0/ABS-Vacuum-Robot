#ifndef SIMULATION
#define SIMULATION

#include "cppyabm/bases.h"
#include <cstdlib>
#include <random>
#include <iostream>
#include <queue>
#include <cmath>
#include <unordered_map>

struct Room;
struct Aspiradora;
struct Tile;

struct Room: public Env<Room, Aspiradora, Tile> /*Env is inherited using template arguments*/ {
    using baseEnv = Env<Room, Aspiradora, Tile>;
    using baseEnv::baseEnv;
    Room(int _numOfAgents) : baseEnv(), numOfAgents(_numOfAgents) {

    }
    virtual shared_ptr<Aspiradora> generate_agent(std::string);
    virtual shared_ptr<Tile> generate_patch(MESH_ITEM);
    void export_csv();
    void setup();
    void spawn_dirt();
    virtual void update();
    virtual void step();
    int numOfAgents;
    int tick = 0;
    std::vector<int> tiles_clean;
    std::vector<int> tiles_dirty;
    std::vector<int> tiles_cleaned;
    inline static std::random_device rd{};
    inline static std::mt19937 gen{rd()};
    inline static std::uniform_int_distribution<int> dist{2, 63};
    std::vector<shared_ptr<Tile>> chrg_stations;
};

struct Tile : public Patch<Room, Aspiradora, Tile> {
    using basePatch = Patch<Room, Aspiradora, Tile>;
    using basePatch::basePatch;
    // any Patch derived class should receive these arguments
    Tile(shared_ptr<Room> env , MESH_ITEM mesh) : basePatch(env,mesh) {
	    this -> setup();
    }

    void setup();
    static double distance(shared_ptr<Tile> a, shared_ptr<Tile> b);

    enum class Type {
	    TILE,
	    CHRG_TILE
    };

    Type type;
    bool dirty;
};

struct Aspiradora : public Agent<Room, Aspiradora, Tile> {
    using baseAgent = Agent<Room, Aspiradora, Tile>;
    using baseAgent::baseAgent;
    // any derived agent should receive these arguments
    Aspiradora(shared_ptr<Room> env, std::string agent_name):baseAgent(env,agent_name) {
	    this->setup();
    }

    void export_csv();
    virtual void setup();
    void update();
    void update_fool();
    void update_memory();
    void update_propagate();
    void update_reactive();
    void update_intelligent();
    virtual void step();
    void step_fool();
    void step_memory();
    void step_propagate();
    void step_reactive();
    void step_intelligent();

    //void move();
    //void clean();
    //void standby();

    std::queue<shared_ptr<Tile>> path_to_tile(shared_ptr<Tile> tile);
    bool neighbor_on_crit_battery();

    enum class State {
	    AVAILABLE,
	    UNAVAILABLE,
	    ON_MISSION,
	    CRITICAL_BATTERY_LEVEL,
	    NO_BATTERY
    };

    enum class Action {
	    STANDBY,
	    MOVE,
	    CLEAN,
	    CHARGE
    };

    enum class Type {
	    FOOL,
	    MEMORY,
	    LEADER,
	    FOLLOWER
    };

    int battery;
    int garbage_bay_level;
    vector<int> battery_history;
    static constexpr int max_garbage_bay_level = 10;
    static constexpr int max_battery = 100;
    static constexpr int min_battery = 0;
    static constexpr int crit_battery = 20;
    static constexpr int opt_battery = 80;

    inline static std::unordered_map<Action, int> cost = {
	    {Action::STANDBY, -1},
	    {Action::MOVE, -3},
	    {Action::CLEAN, -2},
	    {Action::CHARGE, 10}
    };

    State state;
    Action action;
    std::queue<shared_ptr<Tile>> path;
    std::string id;
};

struct Tile_info {
	shared_ptr<Tile> t = nullptr;
	double dist = 0;

	bool operator<(const Tile_info& other) const {
		return dist < other.dist;
	}

	bool operator<=(const Tile_info& other) const {
		return dist <= other.dist;
	}

	bool operator>(const Tile_info& other) const {
		return dist > other.dist;
	}

	bool operator>=(const Tile_info& other) const {
		return dist >= other.dist;
	}

	bool operator==(const Tile_info& other) const {
		return dist == other.dist;
	}
};

shared_ptr<Aspiradora> Room::generate_agent(std::string agent_name) {
    auto agent_obj = make_shared<Aspiradora>(this->shared_from_this(), agent_name);
    this->agents.push_back(agent_obj);
    return agent_obj;
};

shared_ptr<Tile> Room::generate_patch(MESH_ITEM mesh){
    auto patch_obj = make_shared<Tile>(this->shared_from_this(), mesh);
    this->patches.insert(pair<unsigned, shared_ptr<Tile>>(patch_obj->index, patch_obj));
    return patch_obj;
};

inline void Room::export_csv()
{
    std::ofstream file{"analysis/dirt.csv"};

    file << "tick,dirty\n";

    for (std::size_t i = 0; i < tiles_dirty.size(); ++i) {
        file << i << ','
             << tiles_dirty[i] << '\n';
    }

    std::ofstream file2{"analysis/clean.csv"};

    file2 << "tick,clean\n";

    for (std::size_t i = 0; i < tiles_clean.size(); ++i) {
        file2 << i << ','
             << tiles_clean[i] << '\n';
    }

    for (auto& agent : this->agents) {
	    agent->export_csv();
    }
}

inline void Room::setup() {
	auto mesh = space::grid2(8, 8, 1, false);
	this->setup_domain(mesh);

	for (int i = 0; i < numOfAgents; i++) {
		auto vacuum = this->generate_agent("aspiradora");
		this->place_agent(this->patches.at(i), vacuum);
		vacuum->id = std::string("A") + std::to_string(i);
	}

	auto& tile = this->patches.at(28);
	tile->dirty = true;
	auto& charging_station1 = this->patches.at(0);
	charging_station1->type = Tile::Type::CHRG_TILE;
	chrg_stations.push_back(charging_station1);
	auto& charging_station2 = this->patches.at(1);
	charging_station2->type = Tile::Type::CHRG_TILE;
	chrg_stations.push_back(charging_station2);
	this->update();
}


inline void Room::spawn_dirt() {
	unsigned random_index = dist(gen);
	auto tile = this->patches.at(random_index);
	if (tile->dirty) {
		for (auto& [index, patch] : this->patches) {
			if (patch->type == Tile::Type::CHRG_TILE) {
				continue;
			}
			if (!patch->dirty) {
				tile = patch;
				break;
			}
		}
	}
	tile->dirty = true;
}

inline void Room::update() {
	Env<Room, Aspiradora, Tile>::update();
	for (auto& agent : this->agents) {
		agent->update();
		//std::cout << "agent battery: " << agent->battery << std::endl;
	}

	std::cout << "NEW STEP " << tick << std::endl;

	tiles_clean.push_back(0);
	tiles_dirty.push_back(0);
	tiles_cleaned.push_back(0);

	auto& clean = tiles_clean.back();
	auto& dirty = tiles_dirty.back();

	for (auto& [index, patch] : this->patches) {
		if (patch->dirty) {
			dirty += 1;
		} else {
			clean += 1;
		}
	}
}

inline void Room::step() {
	for (unsigned i = 0; i < this->agents.size(); i++) {
		this->agents[i]->step();
	}

	if (tick % 5 == 0) {
		spawn_dirt();
	}

	this->update();
	this->tick++;
}

inline void Tile::setup() {
	type = Tile::Type::TILE;
	dirty = false;
}

inline double Tile::distance(shared_ptr<Tile> a, shared_ptr<Tile> b) {
	return std::sqrt(std::pow(b->coords[0] - a->coords[0], 2) + std::pow(b->coords[1] - a->coords[1], 2));
}

inline void Aspiradora::export_csv() {
	std::ofstream file{std::string("analysis/") + std::string(this->id) + std::string("_battery_level.csv")};

	file << "tick,battery\n";

	for (std::size_t i = 0; i < battery_history.size(); i++) {
		file << i << ',' << battery_history[i] << '\n';
	}
}

inline void Aspiradora::setup() {
	battery = max_battery;
	garbage_bay_level = 0;
	state = Aspiradora::State::AVAILABLE;
	action = Aspiradora::Action::STANDBY;
}

inline void Aspiradora::update() {
	update_propagate();
	battery_history.push_back(battery);
	std::cout << this->id << " -------- " << std::endl;
	std::cout << "State: " << static_cast<int>(state) << " Action: " << static_cast<int>(action) << " Battery: " << battery << std::endl;
}

inline void Aspiradora::update_fool() {
}

inline void Aspiradora::update_memory() {
	switch (state) {
		case Aspiradora::State::AVAILABLE:
			switch (action) {
				case Aspiradora::Action::MOVE: {
				        auto dest_ = this->_patch->empty_neighbor(true);
				        if (dest_) {
				              this->move(dest_, true);
				        }
					this->battery += cost[action];
					}
					break;
				case Aspiradora::Action::STANDBY:
					this->battery += cost[action];
					break;
				default:
					break;
			}
			break;
		case Aspiradora::State::UNAVAILABLE:
			switch (action) {
				case Aspiradora::Action::CHARGE:
					if (!path.empty()) {
						path = std::queue<shared_ptr<Tile>>(); 
					}
					this->battery += cost[action];
					break;
				default:
					break;
			}
			break;
		case Aspiradora::State::ON_MISSION:
			switch (action) {
				case Aspiradora::Action::CLEAN:
					this->_patch->dirty = false;
					this->env->tiles_cleaned.back() += 1;
					this->battery += cost[action];
					break;
				default:
					break;
			}
			break;
		case Aspiradora::State::CRITICAL_BATTERY_LEVEL:
			switch (action) {
				case Aspiradora::Action::MOVE:
					if (path.empty()) {
						path = path_to_tile(this->env->chrg_stations[0]);
					}
					this->move(path.front(), true);
					this->battery += cost[action];
					path.pop();
					break;
				default:
					break;
			}
			break;
		case Aspiradora::State::NO_BATTERY:
			switch (action) {
				case Aspiradora::Action::STANDBY:
					break;
				default:
					break;
			}
			break;
	}

	if (this->battery < 0) { this->battery = 0;}
}

inline void Aspiradora::update_propagate() {
	switch (state) {
		case Aspiradora::State::AVAILABLE:
			switch (action) {
				case Aspiradora::Action::MOVE: {
				        auto dest_ = this->_patch->empty_neighbor(true);
				        if (dest_) {
				              this->move(dest_, true);
				        }
					this->battery += cost[action];
					}
					break;
				case Aspiradora::Action::STANDBY:
					this->battery += cost[action];
					break;
				default:
					break;
			}
			break;
		case Aspiradora::State::UNAVAILABLE:
			switch (action) {
				case Aspiradora::Action::CHARGE:
					if (!path.empty()) {
						path = std::queue<shared_ptr<Tile>>(); 
					}
					this->battery += cost[action];
					break;
				default:
					break;
			}
			break;
		case Aspiradora::State::ON_MISSION:
			switch (action) {
				case Aspiradora::Action::CLEAN:
					this->_patch->dirty = false;
					this->env->tiles_cleaned.back() += 1;
					this->battery += cost[action];
					break;
				default:
					break;
			}
			break;
		case Aspiradora::State::CRITICAL_BATTERY_LEVEL:
			switch (action) {
				case Aspiradora::Action::MOVE:
					if (path.empty()) {
						path = path_to_tile(this->env->chrg_stations[0]);
					}
					this->move(path.front(), true);
					this->battery += cost[action];
					path.pop();
					break;
				default:
					break;
			}
			break;
		case Aspiradora::State::NO_BATTERY:
			switch (action) {
				case Aspiradora::Action::STANDBY:
					break;
				default:
					break;
			}
			break;
	}

	if (this->battery < 0) { this->battery = 0;}
}

inline void Aspiradora::update_reactive() {}

inline void Aspiradora::update_intelligent() {}

inline void Aspiradora::step() {
	step_propagate();
}

inline void Aspiradora::step_fool() {
	// If the current tile is dirty, clean
	if (this->_patch->dirty) {
		this->_patch->dirty = false;
		return;
	}

	this->order_move(nullptr, true, false);
}

inline void Aspiradora::step_memory() {

	if (this->_patch->type != Tile::Type::CHRG_TILE) {
		if (battery <= min_battery) {
		        state = Aspiradora::State::NO_BATTERY;
			action = Aspiradora::Action::STANDBY;
		} else if (battery <= crit_battery) {
		        state = Aspiradora::State::CRITICAL_BATTERY_LEVEL;
			action = Aspiradora::Action::MOVE;
		} else {
		        if (this->_patch->dirty) {
				state = Aspiradora::State::ON_MISSION;
				action = Aspiradora::Action::CLEAN;
			} else {
				state = Aspiradora::State::AVAILABLE;
				action = Aspiradora::Action::MOVE;
			}
		}
	} else {
		if (battery < opt_battery && state != Aspiradora::State::AVAILABLE) {
			state = Aspiradora::State::UNAVAILABLE;
			action = Aspiradora::Action::CHARGE;
		} else {
			state = Aspiradora::State::AVAILABLE;
			action = Aspiradora::Action::MOVE;
		}
	}
}

inline void Aspiradora::step_propagate() {
	if (this->_patch->type != Tile::Type::CHRG_TILE) {
		if (battery <= min_battery) {
		        state = Aspiradora::State::NO_BATTERY;
			action = Aspiradora::Action::STANDBY;
		} else if (battery <= crit_battery || neighbor_on_crit_battery()) {
		        state = Aspiradora::State::CRITICAL_BATTERY_LEVEL;
			action = Aspiradora::Action::MOVE;
		} else {
		        if (this->_patch->dirty) {
				state = Aspiradora::State::ON_MISSION;
				action = Aspiradora::Action::CLEAN;
			} else {
				state = Aspiradora::State::AVAILABLE;
				action = Aspiradora::Action::MOVE;
			}
		}
	} else {
		if (battery < opt_battery) {
			state = Aspiradora::State::UNAVAILABLE;
			action = Aspiradora::Action::CHARGE;
		} else {
			state = Aspiradora::State::AVAILABLE;
			action = Aspiradora::Action::MOVE;
		}
	}
}

inline void Aspiradora::step_reactive() {
}

inline void Aspiradora::step_intelligent() {
}

inline std::queue<shared_ptr<Tile>> Aspiradora::path_to_tile(shared_ptr<Tile> tile) {

	std::queue<shared_ptr<Tile>> p;

	shared_ptr<Tile> curr = this->_patch;

	if (curr == tile) {
		return p; // Whe using this function, check if the return is empty, which means the agent is already on the desired tile
	}

	do {
		std::priority_queue<Tile_info, std::vector<Tile_info>, std::greater<Tile_info>> neighbors_ordered;
		for (auto& neighbor : curr->neighbors) {
			Tile_info tile_info;
			tile_info.t = neighbor;
			tile_info.dist = Tile::distance(neighbor, tile);
			neighbors_ordered.push(tile_info);
		}
		curr = neighbors_ordered.top().t;
		p.push(curr);
	} while (p.back() != tile);

	return p;
}

inline bool Aspiradora::neighbor_on_crit_battery()
{
    auto neighbors = this->_patch->find_neighbor_agents(false);

    for (const auto& neighbor : neighbors) {
        if (neighbor->state == Aspiradora::State::CRITICAL_BATTERY_LEVEL) {
            return true;
        }
    }

    return false;
}

#endif
