#!/usr/bin/env python3

import os
import sys
import platform
import subprocess
import pybind11
from setuptools import setup, Extension

def get_library_dir():
    """
    Dynamically determin the libary directory based on OS, Arch, and Version.
    """

    system = sys.platform
    machine = platform.machine()

    arch_map = {
    'x86_64': 'amd64',
    'AMD64': 'amd64',
    'aarch64': 'arm64',
    'arm64': 'arm64',
    'i386': '32bits',
    'i6886': '32bits'
    }

    arch = arch_map.get(machine, 'unknown')

    base_path = "../libs/API_DRFL/library"

    if system == 'win32':
        # System is Windows
        if arch == 'amd64': #64-bits Windows
            return os.path.join(base_path, 'Windows', '64bits')
        elif arch == '32bits': #32-bits Windows
            return os.path.join(base_path, 'Windows', '32bits')
        else:
            raise ValueError(f"Unsupported Windows architecture: {machine}")

    elif system == 'linux':
        # System is Linux

        dist_name = None
        dist_verion = None

        try:
            import distro
            dist_name = distro.id()
            dist_version = distro.version()
        except ImportError:
            if os.path.exist('/etc/os-release'):
                with open('/etc/os-release') as f:
                    for line in f:
                        if line.startswith('ID='):
                            dist_name = line.split('=')[1].strip().strip('"')
                        elif line.startwith('VERSION_ID='):
                            dist_version = line.split('=')[1].strip().strip('"')

        if dist_name == 'ubuntu':
            # Mapping for specific Ubuntu version
            if dist_version in ['18.04', '20.04', '22.04', '24.04']:
                if arch in ['amd64', 'arm64']:
                    return os.path.join(base_path, 'Linux/64bits', arch, dist_version)
                else:
                    raise ValueError(f"Unsupported Ubuntu architecture: {machine}")
            else:
                raise ValueError(f"Unsupported Ubuntu version: {dist_version}. Supported: 18.04, 20.04, 22.04, 24.04")

        if arch in ['amd64', 'arm64']: # Generic Linux fallback (Not Ubuntu)
            return os.path.join(base_path, 'Linux/64bits', arch, '24.04')
        else:
            raise ValueError(f"Unsupported Linux architecture: {machine}")
    else:
        raise OSError(f"Unsupported operating system: {system}")

try:
    lib_dir = get_library_dir()
    abs_lib_dir = os.path.abspath(lib_dir)
    print(f"Detected library directory: {abs_lib_dir}")
except Exception as e:
    print(f"Error detecting library directory: {e}")
    sys.exit(1)

ext_modules = [
    Extension(
        'doosan_drfl',                      # Name of the generated Python module
        ['../libs/src/drfl_wrapper.cpp'],                # Your pybind11 C++ file
        include_dirs=[
            pybind11.get_include(),
            '../libs/API-DRFL/include'            # Path to Doosan DRFL.h
        ],
        library_dirs=[                      # TODO make variable based on ubuntu version
            abs_lib_dir          # Path to Doosan libDRFL.a / POCO libs
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
    package_data={'doosan_drfl': ['*.pyi']},
    include_package_data=True,
)
