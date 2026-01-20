#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <array>
#include <string>

#include "core/tui.h"

namespace py = pybind11;

// Safe Python-facing wrapper for Label.
//
// IMPORTANT: Label stores a `const std::string&` (non-owning reference).
// If we bound Label directly, a Python string would be converted to a temporary
// std::string that dies right after the call -> dangling reference.
//
// This wrapper owns the std::string so the reference stays valid.
struct PyLabel {
    std::string text;
    Label label;

    PyLabel(Surface& root, std::string t, std::array<int, 2> offset)
        : text(std::move(t)), label(root, text, offset) {}

    void set_text(std::string t) {
        text = std::move(t);
        label.updateSurface();
    }

    const std::string& get_text() const { return text; }

    void update() { label.updateSurface(); }
};

PYBIND11_MODULE(tui, m) {
    m.doc() = "lightweight TUI library (pybind11 bindings)";

    // -----------------
    // Character
    // -----------------
    py::class_<Character>(m, "Character", py::dynamic_attr())
        .def(py::init<>())
        .def("set_bg", &Character::set_bg, py::arg("r"), py::arg("g"), py::arg("b"))
        .def("set_fg", &Character::set_fg, py::arg("r"), py::arg("g"), py::arg("b"))
        .def("set_ch", &Character::set_ch, py::arg("ch"))
        .def("genrate", &Character::genrate)
        // raw fields (handy from Python)
        .def_readwrite("bg_def", &Character::bg_def)
        .def_readwrite("fg_def", &Character::fg_def)
        .def_readwrite("ch_def", &Character::ch_def)
        .def_readwrite("fg", &Character::fg)
        .def_readwrite("bg", &Character::bg)
        .def_readwrite("ch", &Character::ch)
        .def_readwrite("ansi", &Character::ansii);

    // -----------------
    // Surface
    // -----------------
    py::class_<Surface>(m, "Surface", py::dynamic_attr())
        .def(
            py::init([](std::array<int, 2> size,
                        std::string ch,
                        int z,
                        std::array<int, 2> offset) {
                return Surface(size, std::move(ch), z, offset);
            }),
            py::arg("size"),
            py::arg("ch") = " ",
            py::arg("z") = 0,
            py::arg("offset") = std::array<int, 2>{0, 0}
        )
        .def("fill_bg", &Surface::fill_bg, py::arg("r"), py::arg("g"), py::arg("b"))
        .def("fill_fg", &Surface::fill_fg, py::arg("r"), py::arg("g"), py::arg("b"))
        .def("set_z", &Surface::set_z, py::arg("z"))
        .def("get_z", &Surface::get_z)
        .def("set_offset", &Surface::set_offset, py::arg("x"), py::arg("y"))
        .def("offset", &Surface::offset)
        .def("size", &Surface::ssize)
        .def("blit", &Surface::blit, py::arg("surf"))
        .def(
            "__getitem__",
            [](Surface& self, int idx) -> Character& { return self[idx]; },
            py::arg("idx"),
            py::return_value_policy::reference_internal
        );

    // -----------------
    // Screen
    // -----------------
    py::class_<Screen>(m, "Screen", py::dynamic_attr())
        .def(py::init<>())
        .def(
            "append",
            [](Screen& self,
               std::array<int, 2> size,
               std::array<int, 2> offset,
               std::string ch,
               int z) -> Surface& {
                // Screen::append signature: (size, offset, ch, z)
                return self.append(size, offset, std::move(ch), z);
            },
            py::arg("size"),
            py::arg("offset"),
            py::arg("ch") = " ",
            py::arg("z") = -1,
            py::return_value_policy::reference_internal
        )
        .def("get_size", &Screen::getSize)
        .def("flip", &Screen::flip)
        .def_readwrite("amount", &Screen::amount);

    // -----------------
    // Label (safe wrapper)
    // -----------------
    py::class_<PyLabel>(m, "Label", py::dynamic_attr())
        .def(py::init<Surface&, std::string, std::array<int, 2>>(),
             py::arg("root"), py::arg("text"), py::arg("offset"))
        .def("update", &PyLabel::update)
        .def_property("text", &PyLabel::get_text, &PyLabel::set_text);

    // -----------------
    // Input
    // -----------------
    py::class_<Input>(m, "Input", py::dynamic_attr())
        .def(py::init<>())
        .def("get_char", &Input::get_char);
}
