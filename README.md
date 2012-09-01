![logo](https://github.com/claerhout/testrail/raw/master/testrail-logo.png)

TestRail is a unit test framework designed for ANSI C99 compliant programs only.
Platform-specific features are NOT supported (multi-threading in particular).
TestRail is compatible with **Linux** and **OSX**, Windows is planned soon.

-------------------------------------------------------------------------------

1. In your project directory, add testrail as a git submodule: **$ git submodule add https://github.com/claerhout/testrail**
2. Check testrail is working: **$ make -C testrail -f testrail.gmake**, the second to last line should say "testrail done" 
3. Add a test rule to your project makefile.
	That rule should specify the required objects needed by your tests.
	Those objects are then passed on to testrail via the OBJ variable.
	Include paths are specified by the INC variable.
	e.g.:
		.PHONY: test
		test: foo.o
		$(MAKE) -f testrail/testrail.gmake INC=.:testrail OBJ=$^

-------------------------------------------------------------------------------

[![Build Status](https://secure.travis-ci.org/claerhout/testrail.png?branch=master)](http://travis-ci.org/claerhout/testrail)