[![Build Status](https://secure.travis-ci.org/claerhout/testrail.png?branch=master)](http://travis-ci.org/claerhout/testrail)

TestRail is a unit test framework designed for ANSI C99 compliant programs only.
Platform-specific features are NOT supported (multi-threading in particular).
This framework is compatible with **Linux** and **OSX** platforms, Windows support is planned soon.

Usage
-----

* Pull testrail.git to your project directory.
* Place your tests in the **test/** directory.
* Adjust **testrail.gmake** (CFLAGS variable and %.o rule).
* Run the makefile: **make -f testrail.gmake**, all the tests will be compiled and run.

Test Development
----------------

* Read **testrail.h** for an introduction to testrail concepts.
* Check test/example.c for a quick overview.