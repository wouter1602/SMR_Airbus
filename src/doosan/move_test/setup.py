#!/usr/bin/env python3
#
import os
import pybind11
from setuptools import setup, Extension

ext_modules = [
    Extension(
        'doosan_drfl',                      # Name of the generated Python module
        ['../libs/src/drfl_wrapper.cpp'],                # Your pybind11 C++ file
        include_dirs=[
            pybind11.get_include(),
            '../libs/API-DRFL/include'            # Path to Doosan DRFL.h
        ],
        library_dirs=[                      # TODO make variable based on ubuntu version
            '../libs/API-DRFL/library/Linux/64bits/amd64/24.04'          # Path to Doosan libDRFL.a / POCO libs
        ],
        libraries=[
            'DRFL',                         # Links libDRFL.a / .so
            'PocoFoundation',               # Doosan dependency
            'PocoNet'                       # Doosan dependency
        ],
        language='c++',
        extra_compile_args=['-std=c++17']   # Ensure C++17 or higher
    ),
]

setup(
    name='doosan_drfl',
    version='1.0',
    description='Python wrapper for Doosan DRFL API',
    ext_modules=ext_modules,
)
