#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <array>
#include "tui.h"

namespace py = pybind11;

PYBIND11_MODULE(tui, m) {
    m.doc() = "light weight tui libbray";

    // Screen
    py::class_<Screen>(m, "Screen", py::dynamic_attr())
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
            py::arg("offset"),
            py::return_value_policy::reference_internal
        )
        .def("get_size", &Screen::get_size)
        .def("flip", &Screen::flip);

    // Surface
    py::class_<Surface>(m, "Surface", py::dynamic_attr())
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
        .def("add_string", [](Surface &self, string text, int col_start, int row, array<int, 3> fg, array<int, 3> bg){
            int fga[3] = {fg[0], fg[1], fg[2]};
            int bga[3] = {bg[0], bg[1], bg[2]};
            return self.add_string(text, col_start, row, fga, bga);
        },
             py::arg("text"),
             py::arg("col_start"),
             py::arg("row"),
             py::arg("fg"),
             py::arg("bg"))
        .def("set_z", &Surface::set_z, py::arg("z"))
        .def("__getitem__", &Surface::operator[], py::return_value_policy::reference_internal)
        .def("set_offset", &Surface::set_offset,
             py::arg("x"), py::arg("y"));

    py::class_<Character>(m, "Character", py::dynamic_attr())
        .def(py::init<>())
        .def("set_bg", &Character::set_bg, py::arg("r"), py::arg("g"), py::arg("b"))
        .def("set_fg", &Character::set_fg, py::arg("r"), py::arg("g"), py::arg("b"))
        .def("set_ch", &Character::set_ch, py::arg("ch")) 
        .def("genrate", &Character::genrate)
        .def_readwrite("ansi", &Character::ansii);

    py::class_<Input>(m, "Input", py::dynamic_attr())
        .def(py::init<>())
        .def("get_char", &Input::get_char);


}
