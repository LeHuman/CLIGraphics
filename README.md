<!-- PROJECT: CLIGraphics -->
<!-- TITLE: CLIGraphics -->
<!-- KEYWORDS: Graphics, 3D -->
<!-- LANGUAGES: C++ -->
<!-- TECHNOLOGY: glm -->
<!-- STATUS: Inactive -->

<!-- DEMO -->
![DemoGif](<images/demo.gif>)

# CLIGraphics

[About](#about) - [Usage](#usage) - [License](#license)

## Status

**`Inactive`** - *Not actively being worked on. May be revisited at a later date.*

## About

<!-- DESCRIPTION START -->
A crude implementation of a 3D STL viewer completely in a terminal with mouse support.
<!-- DESCRIPTION END -->
This project mainly serves as a way to play around with C++ as is by no means meant to be a good display of graphics programming.

### Why

<!-- WHY START -->
This was meant as a warm up for an interview. Also, I thought it was a neat idea that I wanted to try to do, which has been done before.
<!-- WHY END -->

## Usage

Edit line 28 in the main file to point to the model you want to view. Rebuild and then run.

Depending on your terminal, you may also have to adjust line 16, which is the viewport size in characters.

The stl will be shown rotating around it's origin point, where, if it is detected, moving the mouse affects the camera view.

There is no real zooming in or out but dividing the incoming stl in stlglm.cpp helps with that.

### 3D Models

When building from source, low poly STL files should be included in the root directory under `models`
F:/GitHub/CLIGraphics/build/Debug/CLIGraphics.exe

### Building

Easiest way to test and build is to open this project in VSCode with CMake extensions installed.

#### Build and run the standalone target

Use the following command to build and run the executable target.

```bash
cmake -S standalone -B build/standalone
cmake --build build/standalone
./build/standalone/Greeter --help
```

#### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S test -B build/test
cmake --build build/test
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build/test --target test

# or simply call the executable: 
./build/test/GreeterTests
```

To collect code coverage information, run CMake with the `-DENABLE_TEST_COVERAGE=1` option.

#### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system.

```bash
cmake -S test -B build/test

# view changes
cmake --build build/test --target format

# apply changes
cmake --build build/test --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.
These dependencies can be easily installed using pip.

```bash
pip install clang-format==14.0.6 cmake_format==0.6.11 pyyaml
```

#### Build the documentation

The documentation is automatically built and [published](https://thelartians.github.io/ModernCppStarter) whenever a [GitHub Release](https://help.github.com/en/github/administering-a-repository/managing-releases-in-a-repository) is created.
To manually build documentation, call the following command.

```bash
cmake -S documentation -B build/doc
cmake --build build/doc --target GenerateDocs
# view the docs
open build/doc/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen, jinja2 and Pygments installed on your system.

#### Build everything at once

The project also includes an `all` directory that allows building all targets at the same time.
This is useful during development, as it exposes all subprojects to your IDE and avoids redundant builds of the library.

```bash
cmake -S all -B build
cmake --build build

# run tests
./build/test/GreeterTests
# format code
cmake --build build --target fix-format
# run standalone
./build/standalone/Greeter --help
# build docs
cmake --build build --target GenerateDocs
```

## License

Unlicense
