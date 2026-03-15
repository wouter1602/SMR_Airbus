#define DRCF_VERSION 2

#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/cast.h>

#include "../API-DRFL/include/DRFL.h"

namespace py = pybind11;
using arr_f = py::array_t<float, py::array::c_style | py::array::forcecast>;


// Open connection function
void bOpenConnection(py::class_<DRAFramework::CDRFLEx>& c);
void bCloseConnection(py::class_<DRAFramework::CDRFLEx>& c);
