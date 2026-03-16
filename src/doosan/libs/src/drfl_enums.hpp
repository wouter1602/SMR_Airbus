#pragma once

#include <pybind11/pybind11.h>

namespace py = pybind11;

// Forward declaration – implemented in drfl_enums.cpp
void bind_drfl_enums(py::module_ &m);
