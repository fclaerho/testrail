![logo](https://github.com/claerhout/testrail/raw/master/testrail-logo.png)

TestRail is a unit test framework designed for ANSI C99 compliant programs only.
Platform-specific features are NOT supported (multi-threading in particular).
This framework is compatible with **Linux** and **OSX** platforms, Windows support is planned soon.

1. In your project directory, add testrail as a [git submodule](http://git-scm.com/book/en/Git-Tools-Submodules): **$ git submodule add https://github.com/claerhout/testrail**
2. Check testrail is working: **$ make -C testrail -f testrail.gmake** 
2. (under work)

***
[![Build Status](https://secure.travis-ci.org/claerhout/testrail.png?branch=master)](http://travis-ci.org/claerhout/testrail)