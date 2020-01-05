#include <pybind11/pybind11.h>

#include "util/TowerProperties.hpp"
#include "Model/TowerDefense.hpp"
#include "Model/TowerLogic.hpp"
#include "shared/StubFrontend.hpp"

namespace py = pybind11;



void wrap_gameserver(py::module &pymod) {
    py::class_<TowerDefense<FrontStub, TowerLogic>>(pymod, "TowerDefense")
		.def (py::init<int32_t>())
		.def ("init_game", &TowerDefense<FrontStub, TowerLogic>::init_game)
		.def ("start_game", &TowerDefense<FrontStub, TowerLogic>::start_game)
		.def ("stop_game", &TowerDefense<FrontStub, TowerLogic>::stop_game)
		.def ("get_td_frontend", &TowerDefense<FrontStub, TowerLogic>::get_td_frontend)
		.def ("get_td_backend", &TowerDefense<FrontStub, TowerLogic>::get_td_backend);

	py::class_<FrontStub<TowerLogic>>(pymod, "FrontStub")
		.def ("spawn_build_tower_event", &FrontStub<TowerLogic>::spawn_build_tower_event)
		.def ("spawn_modify_tower_event", &FrontStub<TowerLogic>::spawn_modify_tower_event<tower_properties>)
		.def ("spawn_print_tower_event", &FrontStub<TowerLogic>::spawn_print_tower_event) 
		.def ("spawn_tower_target_event", &FrontStub<TowerLogic>::spawn_tower_target_event);
}

