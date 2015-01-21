//---------------------------------------------------------------------------
/// \file   link_variable.hpp
/// \brief  An external variable that can be used in an expression
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "float_variable.hpp"

namespace rhea
{

/** A link to an external variable.
 *  It is up to you to make sure the linked variable isn't destroyed while the
 *  solver is still using it.
 */
template <typename T>
class link_variable : public abstract_variable
{
public:
    link_variable(T& value)
        : value_{value}
    {
    }

    virtual ~link_variable() {}

    virtual void set_value(double new_value)
    {
        value_ = static_cast<T>(new_value);
    }

    virtual double value() const { return static_cast<double>(value_); }

    virtual int int_value() const
    {
        return static_cast<int>(value_ + (value_ > T(0.0) ? T(0.5) : T(-0.5)));
    }

    virtual std::string to_string() const { return "link"; }

protected:
    T& value_;
};

/** A link to an external integer.
 *  It is up to you to make sure the linked integer isn't destroyed while the
 *  solver is still using it.  The library will keep an internal double for
 *  the calculations, and update the linked integer with its rounded value.
 */
class link_int : public float_variable
{
    typedef abstract_variable super;

public:
    link_int(int& value)
        : float_variable{static_cast<double>(value)}
        , ivalue_{value}
    {
    }

    virtual ~link_int() {}

    virtual void set_value(double new_value)
    {
        value_ = new_value;
        ivalue_ = int_value();
    }

    virtual std::string to_string() const { return "link"; }

protected:
    int& ivalue_;
};

} // namespace rhea
