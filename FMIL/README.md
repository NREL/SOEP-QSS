# FMI Library Build Support

The cmake scripts under `bin` will build and install the FMI Library on Linux and Windows.

Building from the latest FMIL git master (if stable) or release code is generally recommended for SOEP QSS.

## Directories
* bin                   Build scripts
* custom                FMIL source with customizations (not in repo)
* custom.Linux          Customized files for Linux
* custom.Windows        Customized files for Windows
* fmi-library           FMIL git master from https://github.com/modelon-community/fmi-library.git (not in repo)

## Customizations
* CMakeLists.txt:
  * Build the static (not dynamic) libs and sublibs (Windows)
  * Don't build tests
* jm_portability.h: Add extern "C" blocks

## Building
* Update FMIL code before builds from GitHub in fmi-library and merge into custom
* Merge code being used into the custom directory
* Update custom.*Platform* for changes in FMIL code base
* Merge custom.*Platform* into custom source directory
* Do builds in a directory outside the source tree

### Linux
* Run the desired build script from the bld directory: `../bin/Linux/bld.`_compiler_`.`_build_`.sh`

### Windows
* Run the desired build script from the bld directory: `..\bin\Windows\bld.`_compiler_`.`_build_`.bat`
* Need MinGW installed for make for this option to work: -G "MinGW Makefiles"
* The debug zlib library is named zlibd.lib with VC++ and IC++.
  We rename it to zlib.lib in the installation folder to keep things simple.

## Install

The build scripts will install FMIL to directories of this form:

### Linux
* `/opt/FMIL.`_compiler_`.`_build_

### Windows
* `C:\FMIL.`_compiler_`.`_build_

### Headers

The build scripts install these extra/internal FMIL headers on Linux (analogous on Windows):
* `sudo cp ~/Projects/FMIL/custom/src/CAPI/include/FMI2/fmi2_capi.h /opt/FMIL.<Compiler>.<Build>/include/FMI2`
* `sudo cp ~/Projects/FMIL/custom/src/CAPI/src/FMI2/fmi2_capi_impl.h /opt/FMIL.<Compiler>.<Build>/include/src/FMI2`
* `sudo cp ~/Projects/FMIL/custom/src/Import/src/FMI/fmi_import_context_impl.h /opt/FMIL.<Compiler>.<Build>/include/FMI`
* `sudo cp ~/Projects/FMIL/custom/src/Import/src/FMI2/fmi2_import_impl.h /opt/FMIL.<Compiler>.<Build>/include/FMI2`
* `sudo cp ~/Projects/FMIL/custom/src/XML/include/FMI/*.h /opt/FMIL.<Compiler>.<Build>/include/FMI`
* `sudo cp ~/Projects/FMIL/custom/src/XML/include/FMI1/*.h /opt/FMIL.<Compiler>.<Build>/include/FMI1`
* `sudo cp ~/Projects/FMIL/custom/src/XML/include/FMI2/*.h /opt/FMIL.<Compiler>.<Build>/include/FMI2`
* `sudo cp ~/Projects/FMIL/custom/ThirdParty/Expat/expat-2.1.0/lib/expat*.h /opt/FMIL.<Compiler>.<Build>/include`

## Compilers

### GCC
* FMIL CMake build will build static library with -fPIC by default (FMILIB_BUILD_FOR_SHARED_LIBS) so it can be linked into a shared library
* Windows:
  * The latest WinLibs MinGW or Equation.com GCC compilers should work
  * OCT ships with TDM GCC 5.1 that works (you may see some warnings)

### Visual C++
* Visual C++ 2017 and 2019 work
* CMake uses /MD instead of /MDd for debug builds for some reason
* Get some warnings: Seem mostly like harmless 64-bit portability warnings (size_t conversion to int, ...)

### Intel C++
* Intel C++ oneAPI (2021), 19 (2019), and 19.1 (2020) work
* IX builds use Clang-based icx compiler

## Linking

All the libs are installed by the make install process.
Applications must link in all of the static libs they use.
The link order of the libs matters to avoid undefined references.

## Resources
* https://github.com/modelon-community/fmi-library.git

