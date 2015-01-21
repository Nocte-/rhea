//---------------------------------------------------------------------------
/// \file   float_variable.hpp
/// \brief  A floating point variable that can be used in an expression
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "abstract_variable.hpp"
namespace rhea
{

/** A floating-point variable. */
class float_variable : public abstract_variable
{
public:
    float_variable(double value = 0.0)
        : value_{value}
    {
    }

    virtual ~float_variable() {}

    virtual double value() const { return value_; }

    virtual int int_value() const
    {
        return static_cast<int>(value_ + (value_ > 0.0 ? 0.5 : -0.5));
    }

    virtual void set_value(double v) { value_ = v; }

    virtual std::string to_string() const { return "var"; }

protected:
    double value_;
};

} // namespace rhea
