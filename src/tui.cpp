
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <array>
#include "tui.h"

namespace py = pybind11;

PYBIND11_MODULE(tui, m) {
    m.doc() = "tui bindings";

    // Screen
    py::class_<Screen>(m, "Screen")
        .def(py::init<>())
        .def(
            "append",
            [](Screen &self,
               std::array<int, 2> size,
               const std::string &ch,
               int z,
               std::array<int, 2> offset) -> Surface& {
                int size_arr[2]   = { size[0],   size[1] };
                int offset_arr[2] = { offset[0], offset[1] };
                return self.append(size_arr, ch, z, offset_arr);
            },
            py::arg("size"),
            py::arg("ch"),
            py::arg("z"),
            py::arg("offset")
        )
        .def("flip", &Screen::flip);

    // Surface
    py::class_<Surface>(m, "Surface")
        .def(
            py::init([](std::array<int, 2> size,
                        const std::string &ch,
                        int z,
                        std::array<int, 2> offset) {
                int size_arr[2]   = { size[0],   size[1] };
                int offset_arr[2] = { offset[0], offset[1] };
                return new Surface(size_arr, ch, z, offset_arr);
            }),
            py::arg("size"),
            py::arg("ch"),
            py::arg("z"),
            py::arg("offset")
        )
        .def("fill_bg", &Surface::fill_bg,
             py::arg("r"), py::arg("g"), py::arg("b"))
        .def("fill_fg", &Surface::fill_fg,
             py::arg("r"), py::arg("g"), py::arg("b"))
        .def("add_string", &Surface::add_string,
             py::arg("text"),
             py::arg("col_start"),
             py::arg("row"),
             py::arg("fg"),
             py::arg("bg"))
        .def("set_z", &Surface::set_z, py::arg("z"))
        .def("set_offset", &Surface::set_offset,
             py::arg("x"), py::arg("y"));
}
