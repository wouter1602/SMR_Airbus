#pragma once

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <cstring>
#include <stdexcept>
#include <string>

namespace py = pybind11;

// ─────────────────────────────────────────────────────────────────────────────
// Helper functions: Make a NumPy array that is a copy of a fixed-size C array
// member. Used for read access when you want a detached NumPy array.
// ─────────────────────────────────────────────────────────────────────────────
template <typename T, std::size_t N>
py::array_t<T> array_copy(const T (&arr)[N]) {
  auto result = py::array_t<T>(N);
  std::memcpy(result.mutable_data(), arr, sizeof(T) * N);
  return result;
}

// 2-D version
template <typename T, std::size_t R, std::size_t C>
py::array_t<T> array2d_copy(const T (&arr)[R][C]) {
  auto result = py::array_t<T>({(py::ssize_t)R, (py::ssize_t)C});
  std::memcpy(result.mutable_data(), &arr[0][0], sizeof(T) * R * C);
  return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: set a fixed-size C array member from a NumPy array, with bounds
// check.
// ─────────────────────────────────────────────────────────────────────────────
template <typename T, std::size_t N>
void array_set(T (&arr)[N], py::array_t<T> src) {
  auto info = src.request();
  if (info.size != static_cast<py::ssize_t>(N))
    throw std::out_of_range("Array size mismatch: expected " +
                            std::to_string(N) + ", got " +
                            std::to_string(info.size));
  std::memcpy(arr, info.ptr, sizeof(T) * N);
}

template <typename T, std::size_t R, std::size_t C>
void array2d_set(T (&arr)[R][C], py::array_t<T> src) {
  auto info = src.request();
  if (info.size != static_cast<py::ssize_t>(R * C))
    throw std::out_of_range("2D array size mismatch: expected " +
                            std::to_string(R * C) + ", got " +
                            std::to_string(info.size));
  std::memcpy(&arr[0][0], info.ptr, sizeof(T) * R * C);
}

// ─────────────────────────────────────────────────────────────────────────────
// Macro shortcuts
//
// Root cause of the two compiler errors:
//
//  ERROR 1 – STR_PROP / ARRAY_PROP used `auto &s` in the SETTER lambda.
//    pybind11's strip_function_object needs to call
//    decltype(&Lambda::operator()) to deduce the function signature.  When
//    operator() is a template (because of `auto` parameters) that decltype is
//    ambiguous → "cannot resolve address of overloaded function". FIX: every
//    lambda parameter must be a fully concrete, non-deduced type. We achieve
//    this with a Struct template parameter on the macro + an explicit cast
//    inside, but the cleanest approach (zero overhead, works with pybind11) is
//    to use typed free-function helpers and wrap them in non-generic lambdas
//    via ARRAY_PROP_T / STR_PROP_T macros that take the struct type explicitly.
//
//  ERROR 2 – ARRAY_PROP setter used `decltype(s.member)` in the lambda
//    parameter type.  When multiple structs have a member with the same name
//    (e.g. `_fTargetPos`, `_iOverride`) the compiler generates lambdas whose
//    mangled names are identical → "mangling conflicts with a previous mangle".
//    FIX: pass the concrete element type (float, unsigned char, …) as an
//    explicit macro argument instead of deducing it from the member.
//
// The macros below require the concrete struct type (S) and element type (T)
// to be supplied explicitly, which gives every lambda a unique, fully-specified
// non-template operator() and avoids all mangling collisions.
// ─────────────────────────────────────────────────────────────────────────────

// 1-D array property: ARRAY_PROP(StructType, member, ElementType)
#define ARRAY_PROP(S, member, T)                                               \
  .def_property(                                                               \
      #member, [](const S &s) { return array_copy(s.member); },                \
      [](S &s, py::array_t<T> v) { array_set(s.member, v); })

// 2-D array property: ARRAY2D_PROP(StructType, member, ElementType)
#define ARRAY2D_PROP(S, member, T)                                             \
  .def_property(                                                               \
      #member, [](const S &s) { return array2d_copy(s.member); },              \
      [](S &s, py::array_t<T> v) { array2d_set(s.member, v); })

// String (char[]) property: STR_PROP(StructType, member)
#define STR_PROP(S, member)                                                    \
  .def_property(                                                               \
      #member, [](const S &s) { return std::string(s.member); },               \
      [](S &s, const std::string &v) {                                         \
        std::strncpy(s.member, v.c_str(), sizeof(s.member) - 1);               \
        s.member[sizeof(s.member) - 1] = '\0';                                 \
      })

// ─────────────────────────────────────────────────────────────────────────────
// Forward declaration – implemented in drfl_structs.cpp
// ─────────────────────────────────────────────────────────────────────────────
void bind_drfl_structs(py::module_ &m);
