#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>


namespace py = pybind11;

void wrap_events(py::module &);
void wrap_modifiers(py::module &);

PYBIND11_MODULE(pyDTD, pymod) {
    wrap_events(pymod);
    wrap_modifiers(pymod);
}
