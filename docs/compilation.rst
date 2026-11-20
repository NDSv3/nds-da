.. SPDX-License-Identifier: Apache-2.0
.. Copyright 2026 I2A2-UPM

Build and install
=================

To compile and install the library, follow these steps:

#. Clone the repository and navigate to the project directory:

   ..  code-block:: console

       git clone <repository_url>
       cd nds-deviceaccess

#. Initialize the CMake build system and build the project:

   ..  code-block:: console

       cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
       cmake --build build -- -j$(nproc)
       cmake --build build --target package

#. Install the library:

   ..  code-block:: console

       sudo dnf install build/<package-name>.rpm

Unit tests
----------

To run the unit tests, use the ``test`` CMake target:

..  code-block:: console

    cmake --build build --target test

Coverage report
---------------

To generate code coverage reports, compile the library with coverage enabled and use the ``coverage`` CMake target:

..  code-block:: console

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
    cmake --build build --target coverage

The results can then be found in the ``build/coverage`` directory, in detailed HTML, nested HTML and cobertura XML formats.