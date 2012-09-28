//---------------------------------------------------------------------------
/// \file   floatvariable.hpp
/// \brief  A floating point variable that can be used in an expression
//
// This file is part of Rhea.  Rhea is free software: you can redistribute
// it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3
// of the License, or (at your option) any later version.
//
// Rhea is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Rhea.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include "abstract_variable.hpp"

namespace rhea {

template <typename t>
class pod_variable : public abstract_variable
{
    typedef abstract_variable super;

public:
    pod_variable()
        : abstract_variable()
    { }

    pod_variable(t value)
        : abstract_variable()
        , value_(value)
    { }

    pod_variable(std::string name, t value)
        : abstract_variable(name)
        , value_(value)
    { }

    virtual ~pod_variable() { }

    virtual bool is_dummy() const { return false; }
    virtual bool is_external() const { return true; }
    virtual bool is_pivotable() const { return false; }
    virtual bool is_restricted() const { return false; }

    virtual void set_value(t new_value)
        { value_ = new_value; }

    virtual void change_value(t new_value)
        { value_ = new_value; }

    virtual std::string to_string() const { return std::to_string(value_); }

private:
    // Shouldn't be passed by value
    pod_variable(const pod_variable<t>& copy) {}

protected:
    t value_;
};

class float_variable : public pod_variable<double>
{
public:
    float_variable() : pod_variable(0.0)
    { }

    float_variable(double value) : pod_variable(value)
    { }

    float_variable(std::string name, double value) : pod_variable(name, value)
    { }

    virtual bool is_float() const { return true; }

    virtual double value() const
        { return value_; }

    virtual int int_value() const
        { return static_cast<int>(value_ + (value_ > 0.0 ? 0.5 : -0.5)); }
};

} // namespace rhea

