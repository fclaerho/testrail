![logo](https://github.com/claerhout/testrail/raw/master/testrail-logo.png)

TestRail is a unit test framework designed for ANSI C99 compliant programs only.
Platform-specific features are NOT supported (multi-threading in particular).
TestRail is compatible with **Linux** and **OSX**, Windows is planned soon.
Suggestions are welcomed!

-------------------------------------------------------------------------------

1. In your project directory,
   if you use git as VCS, add testrail as a git submodule:
   **$ git submodule add https://github.com/claerhout/testrail**.
   Remind your users to initialise this module after pulling your project:
   **$ git submodule init** and **$ git submodule update**.
   If you use another VCS, simply pull testrail:
   **$ git pull https://github.com/claerhout/testrail**
2. Check testrail is working: **$ make -C testrail -f testrail.gmake**,
   the second to last line should say "testrail done".
3. Create a **test/** directory and place your tests in there.
   Read **testrail.h** for an [introduction to testrail](https://github.com/claerhout/testrail/blob/master/testrail.h) concepts.
   Read **test/example.h** for some [sample code](https://github.com/claerhout/testrail/blob/master/test/example.c).
4. Add a test rule to your project makefile.
   The prerequisites of that rule are the objects needed to link your tests.
   Those objects are then passed on to the testrail makefile via the OBJ variable.
   Include paths are specified by the INC variable, where entries are separated by a colon.
   e.g.:

		...
		test: foo.o; $(MAKE) -f testrail/testrail.gmake INC=.:testrail OBJ=$^
5. Use **$ make test** to run your tests.
   If they all pass, all intermediary objects and binaries are cleaned up.

-------------------------------------------------------------------------------

[![Build Status](https://secure.travis-ci.org/claerhout/testrail.png?branch=master)](http://travis-ci.org/claerhout/testrail)
TestRail is tested by itself :)