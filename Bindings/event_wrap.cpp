#include <pybind11/pybind11.h>

#include "util/TDEventTypes.hpp"
#include "Model/TowerLogic.hpp"
#include "util/TowerProperties.hpp"

namespace py = pybind11;

namespace UserTowerEvents {
template <typename backend_t>
class PyTowerEvent : public tower_event<backend_t> {
public:
    /* Inherit the constructors */
    using tower_event<backend_t>::tower_event;

    /* Trampoline (need one for each virtual function) */
    void apply(backend_t* td_backend) override {
        PYBIND11_OVERLOAD_PURE(
            void, /* Return type */
            tower_event<backend_t>,      /* Parent class */
            apply,          /* Name of function in C++ (must match Python name) */
            td_backend      /* Argument(s) */
        );
    }
};
}
void wrap_events(py::module &pymod) {
    using backend_t = TowerLogic;
	//TODO: this is a pure virtual class, so I need to handle it differently. See
	//https://pybind11.readthedocs.io/en/master/advanced/classes.html
	py::class_<UserTowerEvents::tower_event<backend_t>, UserTowerEvents::PyTowerEvent<backend_t>>(pymod, "tower_event")
		.def (py::init<>())
		.def (py::init<float, float>());

	py::class_<UserTowerEvents::build_tower_event<backend_t>, UserTowerEvents::tower_event<backend_t>>(pymod, "build_tower_event")
		.def (py::init<>())
        .def (py::init<uint32_t, int, float, float>());

	py::class_<UserTowerEvents::print_tower_event<backend_t>, UserTowerEvents::tower_event<backend_t>>(pymod, "print_tower_event")
		.def (py::init<>())
        .def (py::init<float, float>());

	py::class_<UserTowerEvents::tower_target_event<backend_t>, UserTowerEvents::tower_event<backend_t>>(pymod, "tower_target_event")
		.def (py::init<>())
        .def (py::init<float, float, float, float>());

	py::class_<UserTowerEvents::modify_tower_event<tower_properties, backend_t>, UserTowerEvents::tower_event<backend_t>>(pymod, "modify_tower_event")
		.def (py::init<>())
        .def (py::init<tower_properties, float, float>());
}
