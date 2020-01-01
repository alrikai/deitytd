#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include "util/TowerProperties.hpp"
#include "util/Elements.hpp"


namespace py = pybind11;

void wrap_modifiers(py::module &pymod) {

	py::class_<tower_properties>(pymod, "tower_properties")
		.def (py::init<>())
        .def ("apply_property_modifier", &tower_properties::apply_property_modifier)
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def ("__repr__", [](const tower_properties& tprops) {
            std::ostringstream ostr;
            ostr << tprops;
            return ostr.str();
        });

    py::class_<range<float>>(pymod, "range")
        .def(py::init<>())
        .def(py::init<float, float>())
        .def(py::init<range<float>>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self + float())
        .def(py::self += float())
        .def(py::self * float())
        .def(py::self *= float())
        .def("__repr__", [](const range<float>& val) {
            std::ostringstream ostr;
            ostr << "(" << val.low << ", " << val.high << ")";
            return ostr.str();
        });

    py::class_<tower_property_modifier>(pymod, "tower_property_modifier")
		.def (py::init<>())
        .def ("merge", &tower_property_modifier::merge)
        .def_readwrite("damage_value", &tower_property_modifier::damage_value)
        .def_readwrite("enhanced_damage_value", &tower_property_modifier::enhanced_damage_value)
        .def_readwrite("enhanced_damage_affinity", &tower_property_modifier::enhanced_damage_affinity)
        .def_readwrite("added_damage_value", &tower_property_modifier::added_damage_value)
        .def_readwrite("armor_pierce_damage", &tower_property_modifier::armor_pierce_damage)
        .def_readwrite("enhanced_speed_value", &tower_property_modifier::enhanced_speed_value)
        .def_readwrite("attack_speed_value", &tower_property_modifier::attack_speed_value)
        .def_readwrite("attack_range_value", &tower_property_modifier::attack_range_value)
        .def_readwrite("crit_chance_value", &tower_property_modifier::crit_chance_value)
        .def_readwrite("crit_multiplier_value", &tower_property_modifier::crit_multiplier_value)
        .def("__repr__", [](const tower_property_modifier& val) {
            std::ostringstream ostr;
            //TODO: these are arrays, so we need to loop over the elements
            //ostr << "damage: (" << val.damage_value.low << ", " << val.damage_value..high << ")" << "/n";
            //ostr << "enhanced_damage_value: " << val.enhanced_damage_value << "/n";
            //ostr << "enhanced_damage_affinity: " << val.enhanced_damage_affinity << "/n";
            //ostr << "added_damage_value: " << val.added_damage_value << "/n";
            ostr << "armor_pierce_damage: " << val.armor_pierce_damage << "/n";
            ostr << "enhanced_speed_value: " << val.enhanced_speed_value << "/n";
            ostr << "attack_speed_value: " << val.attack_speed_value << "/n";
            ostr << "attack_range_value: " << val.attack_range_value << "/n";
            ostr << "crit_chance_value: " << val.crit_chance_value << "/n";
            ostr << "crit_multiplier_value: " << val.crit_multiplier_value << "/n";
            return ostr.str();
        });

  //std::vector<event_attribute_modifier *> on_attack_events;
  //std::vector<event_attribute_modifier *> on_hit_events;
  //std::vector<event_attribute_modifier *> on_death_events;
}
