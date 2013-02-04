Rhea
====

About
-----
Rhea is an incremental constraint solver based on 
[Cassowary](http://www.cs.washington.edu/research/constraints/cassowary), 
originally developed by Greg J. Badros and Alan Borning.  The main
differences are:

 * Allows the programmer to write constraints in a natural way
 * Rewritten in C++11, fixes some bugs and memory leaks
 * CMake instead of GNU Autoconfig
 * Unit tests use the Boost Test Framework
 * Uses Doxygen for documentation
 * Expression parser based on Boost Spirit
 * Does not have a finite domain subsolver 


Quick example
-------------

```c++
#include <rhea/simplex_solver.hpp>
#include <rhea/iostream.hpp>

main()
{
    rhea::variable x (0), y (0);
    rhea::simplex_solver solver;

    solver.add_constraints(
    {
        x <= y,
        y == x + 3,
        x == 10
    });

    std::cout << x << " " << y << std::endl;
    // Prints "10 13"
}
```


Status
------
This software is alpha.  It does pass all unit tests, but it hasn't been used
in an actual application yet.  Use at your own peril.

License
-------
Rhea is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License version 3.

Build status
------------
[![Build status](https://travis-ci.org/Nocte-/rhea.png?branch=master)](https://travis-ci.org/Nocte-/rhea)

This project uses [Travis CI](http://travis-ci.org/) to build and run the unit
tests on different compilers automatically.

