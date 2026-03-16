#!/usr/bin/env python3
from setuptools.config.expand import cmdclass

import os
import sys
import platform
import subprocess
import pybind11
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

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

    base_path = "../libs/API-DRFL/library"

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
        dist_version = None

        try:
            import distro
            dist_name = distro.id()
            dist_version = distro.version()
        except ImportError:
            if os.path.exists('/etc/os-release'):
                with open('/etc/os-release') as f:
                    for line in f:
                        if line.startswith('ID='):
                            dist_name = line.split('=')[1].strip().strip('"')
                        elif line.startswith('VERSION_ID='):
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
            return os.path.join(base_path, 'Linux/64bits', arch, 'generic')
        else:
            raise ValueError(f"Unsupported Linux architecture: {machine}")
    else:
        raise OSError(f"Unsupported operating system: {system}")

class BuildExtWithStubs(build_ext):
    """
    Custom build_ext that generates .pyi stubs after compilation.
    """

    def run(self):
        # First, run the standard build_ext
        super().run()

        module_name = 'doosan_drfl'

        self.generate_stubs(module_name)

    def generate_stubs(self, module_name):
        """
        Attempt to generate .pyi stubs using available tools.
        Falls back gracefully if tools aren't available.
        """

        stub_generators = [
            ('pybind11_stubgen', 'pybind11-stubgen'), # Best option
            ('stubgen', 'mypy') # Fallbacck
        ]

        stub_file = f"{module_name}.pyi"
        stub_dir = os.path.join(os.path.dirname(__file__), 'stubs')

        for generator, package_name in stub_generators:
            try:
                # Check if the tool is available
                result = subprocess.run(
                    [ sys.executable, '-m', generator, '--help'],
                    capture_output=True,
                    text=True,
                    timeout=10
                )
                if result.returncode == 0:
                    print(f"\n{'='*60}")
                    print(f"Generating stubs using {generator}...")
                    print(f"\n{'='*60}")

                    os.makedirs(stub_dir, exist_ok=True) #Create stubs dir if not exist

                    if generator == 'pybind11_stubgen':
                        cmd = [
                            sys.executable, '-m', generator,
                            module_name,
                            '-o', stub_dir,
                        ]
                    else: # stubgen
                        cmd = [
                            sys.executable, '-m', generator,
                            '-m', module_name,
                            '-o', stub_dir
                        ]

                    result = subprocess.run(cmd, capture_output=True, text=True)

                    if result.returncode == 0:
                        print(f"✔️ Successfully generated {stub_file}")
                        print(f"  Location: {stub_dir}/{stub_file}")

                        # Move the stub file to the Module direcotry for IDe discorvery
                        src_stub = os.path.join(stub_dir, stub_file)
                        dst_stub = os.path.join(os.path.dirname(__file__), stub_file)

                        if os.path.exists(src_stub):
                            import shutil
                            shutil.copy2(src_stub, dst_stub)
                            print(f"  Copied to: {dst_stub}")

                        return True

                    else:
                        print(f"❌ {generator} failed:")
                        print(result.stderr)
            except (subprocess.TimeoutExpired, FileNotFoundError, Exception) as e:
                print(f"⚠️ {generator} not available or failed: {e}")
                continue
        print(f"\n⚠️ Could not generate stubs automatically.")
        print(f"  To enable automatic stub generation, install one of:")
        print(f"    pip install pybind11-stubgen (preffered)")
        print(f"    pip install mypy             (fallback)")
        print(f"\n   Then run: python setup.py build-ext --inplace")

        return False

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
        ['../libs/src/drfl_wrapper.cpp',
         '../libs/src/cdrflex_bindings.cpp',
         '../libs/src/drfl_structs.cpp'],                # Your pybind11 C++ file
        include_dirs=[
            pybind11.get_include(),
            '../libs/API-DRFL/include'            # Path to Doosan DRFL.h
        ],
        library_dirs=[                      # TODO make variable based on ubuntu version
            abs_lib_dir,          # Path to Doosan libDRFL.a / POCO libs
        ],
        libraries=[
            'DRFL',                         # Links libDRFL.a / .so
            'PocoFoundation',               # Doosan dependency
            'PocoNet',                      # Doosan dependency
        ],
        language='c++',
        extra_compile_args=['-std=c++17'],   # Ensure C++17 or higher
    ),
]

setup(
    name='doosan_drfl',
    version='1.0',
    description='Python wrapper for Doosan DRFL API',
    ext_modules=ext_modules,
    cmdclass={
        'build_ext': BuildExtWithStubs,
    },
    package_data={'doosan_drfl': ['*.pyi']},
    include_package_data=True,
)
