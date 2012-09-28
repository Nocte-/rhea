//---------------------------------------------------------------------------
/// \file   abstract_variable.hpp
/// \brief  Base class for variables
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

#include <cassert>
#include <string>
#include "errors.hpp"

namespace rhea {

class abstract_variable
{
public:
    abstract_variable()
        : name_(std::to_string(++var_nr_))
    { }

    abstract_variable(std::string name)
        : name_(name)
    {
        ++var_nr_;
    }

    virtual ~abstract_variable() { }

    std::string name() const { return name_; }

    virtual void set_name(const std::string& new_name)
        { name_ = new_name; }

    virtual bool is_float() const
        { return false; }

    virtual bool is_fd() const
        { return false; }

    // Return true if this a dummy variable (used as a marker variable
    // for required equality constraints).  Such variables aren't
    // allowed to enter the basis when pivoting.
    virtual bool is_dummy() const
        { return false; }

    // Return true if this a variable known outside the solver.
    // (We need to give such variables a Value after solving is complete.)
    virtual bool is_external() const
        { return false; }

    // Return true if we can Pivot on this variable.
    virtual bool is_pivotable() const
        { throw too_difficult("variable not usable inside simplex_solver"); }

    // Return true if this is a restricted (or slack) variable.  Such
    // variables are constrained to be non-negative and occur only
    // internally to the simplex solver.
    virtual bool is_restricted() const
        { throw too_difficult("variable not usable inside simplex_solver"); }

    virtual double value() const
        { return 0.0; }

    virtual int int_value() const
        { return 0; }

    virtual void set_value(double)
        { assert(false); }

    virtual void change_value(double)
        { assert(false); }

    virtual std::string to_string() const = 0;

private:
    std::string name_;
    static size_t var_nr_;
};

} // namespace rhea

