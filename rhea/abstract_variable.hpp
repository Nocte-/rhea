//---------------------------------------------------------------------------
/// \file   abstract_variable.hpp
/// \brief  Base class for variables
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <string>
#include "errors.hpp"

namespace rhea
{

/** Base class for variables. */
class abstract_variable
{
public:
    virtual ~abstract_variable() {}

    /** Get the value of this variable. */
    virtual double value() const = 0;

    /** Get the value of this variable as an integer */
    virtual int int_value() const = 0;

    virtual void set_value(double) = 0;

    /** Get the value as a string. */
    virtual std::string to_string() const = 0;
};

} // namespace rhea
