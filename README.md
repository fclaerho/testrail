![logo](https://github.com/fclaerho/testrail/raw/master/testrail-logo.png)

**TestRail** is a C unit test framework:
* Strictly ISO C99 compliant ✔
* Supporting tests composition ✔
* Small & simple ✔
* As pictured above, full of awesomeness (wut?) ✔

-------------------------------------------------------------------------------

HOW-TO
------

1. In your project directory,
   * if you use git as VCS, add testrail as a git submodule:
     `$ git submodule add https://github.com/claerhout/testrail.git`.
     Remind your users to initialize this module after pulling your project:
     `$ git submodule init` and `$ git submodule update`.
   * If you use another VCS, simply pull testrail:
     `$ git pull https://github.com/claerhout/testrail.git`.
2. Check testrail is working: `$ make -C testrail -f testrail.gmake`,
   the second to last line should say "testrail done".
3. Create a `test/` directory and place your tests in there.
   Read `testrail.h` for an [introduction to testrail](https://github.com/claerhout/testrail/blob/master/testrail.h) concepts.
   Read `test/example.h` for some [sample code](https://github.com/claerhout/testrail/blob/master/test/example.c).
4. Add a rule for testing to your project makefile, name it as you wish.
   The prerequisites of that rule are the objects needed to link your tests.
   Those objects are then passed on to the testrail makefile via the `OBJ` variable.
   Simply passing `OBJ` as follow should work most of the time: `OBJ="$^"`.
   Be sure to enclose the `$^` variable between quotes so that multiple entries are interpreted as a single token.
   Search paths (sources and headers) are specified by the `VPATH` variable, where entries are separated by a colon.
   e.g.:

		...
		test: foo.o; $(MAKE) -f testrail/testrail.gmake VPATH=.:testrail OBJ="$^"
		...

5. Use `$ make test` to run your tests (if you named the rule as "test").
   If they all pass, all intermediary objects and binaries are cleaned up.

