#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>


namespace py = pybind11;

void wrap_events(py::module &);
void wrap_modifiers(py::module &);
void wrap_gameserver(py::module &);
void wrap_playerstate(py::module &);

PYBIND11_MODULE(_pyDTD, pymod) {
	pymod.doc() = R"pbdoc(
        DeityTD python wrapper -- the best way to develop the worst game 
        ----------------------------------------------------------------

        .. currentmodule:: _pyDTD

        .. autosummary::
           :toctree: _generate
    )pbdoc";

    wrap_events(pymod);
    wrap_modifiers(pymod);
    wrap_gameserver(pymod);
    wrap_playerstate(pymod);
	
#ifdef VERSION_INFO
    pymod.attr("__version__") = VERSION_INFO;
#else
    pymod.attr("__version__") = "dev";
#endif
}
