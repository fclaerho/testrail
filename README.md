TestRail
========

Yet another C unit test framework.

HOWTO
-----

* Copy **testrail.h**, **testrail.c** and **testrail.gmake** into your project directory.
* Create a **test/** directory and place your tests in there.
* Update **testrail.gmake** to specify the dependencies of your project.
* Run the makefile: **make -f testrail.gmake**
* All the test will be compiled and run.
