#include <pybind11/pybind11.h>

#include "shared/Player.hpp"
#include "shared/PlayerInventory.hpp"

#include <sstream>

namespace py = pybind11;

void wrap_playerstate(py::module &pymod) {
	py::class_<TDPlayerInformation>(pymod, "TDPlayerInformation")
		.def (py::init<int,int,int>())
        .def ("get_num_lives", &TDPlayerInformation::get_num_lives)
        .def ("get_num_gold", &TDPlayerInformation::get_num_gold)
        .def ("get_num_essence", &TDPlayerInformation::get_num_essence)
        .def ("lose_life", &TDPlayerInformation::lose_life)
        .def ("gain_life", &TDPlayerInformation::gain_life)
        .def ("update_gold", &TDPlayerInformation::update_gold)
        .def ("update_essence", &TDPlayerInformation::update_essence)
		.def ("add_item", &TDPlayerInformation::add_item)
		.def ("get_inventory_state", &TDPlayerInformation::get_inventory_state)
        .def ("__repr__", [](const TDPlayerInformation& player) {
            std::ostringstream ostr;
            ostr << "lives: " << player.num_lives << "\n";
            ostr << "essence: " << player.num_essence << "\n";
            ostr << "gold: " << player.num_gold << "\n";
            ostr << "inventory: " << player.inventory << "\n";
            return ostr.str();
        });
	
	py::class_<InventoryMetadata>(pymod, "InventoryMetadata")
		.def (py::init<>())
		.def (py::init<std::string>())
		.def ("__repr__", [](const InventoryMetadata& meta) {
            std::ostringstream ostr;
            ostr << meta << "\n";
            return ostr.str();
		});
    
	py::class_<PlayerInventory>(pymod, "PlayerInventory")
		.def (py::init<>())
		.def ("add_item", &PlayerInventory::add_item) 
		.def ("swap_item", &PlayerInventory::swap_item) 
		.def ("get_item", &PlayerInventory::get_item) 
		.def ("remove_item", &PlayerInventory::remove_item) 
		.def_readwrite ("inventory_data", &PlayerInventory::inventory_data)
		.def_readwrite ("inventory_occupied", &PlayerInventory::inventory_occupied)
		.def ("__repr__", [](const PlayerInventory& inventory) {
            std::ostringstream ostr;
            ostr << "player inventory: " << inventory << "\n";
            return ostr.str();
		});
    

}

