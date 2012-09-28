//---------------------------------------------------------------------------
/// \file   constraint.hpp
/// \brief  Base class for constraints
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

#include <memory>
#include <string>
#include "linear_expression.hpp"
#include "strength.hpp"
#include "variable.hpp"

namespace rhea {

class solver;

class constraint
{
    friend class solver;
    friend class simplex_solver;

public:
    constraint(strength str = strength::required(), double weight = 1.0)
        : strength_(str)
        , weight_(weight)
        , times_added_(0)
    { }

    virtual ~constraint() { }

    virtual linear_expression expression() const = 0;

    virtual bool is_edit_constraint() const
        { return false; }

    virtual bool is_inequality() const
        { return false; }

    virtual bool is_strict_inequality() const
        { return false; }

    virtual bool is_required() const
        { return strength_.is_required(); }

    virtual bool is_stay_constraint() const
        { return false; }

    const strength& get_strength() const
        { return strength_; }

    virtual double weight() const
        { return weight_; }

    const variable_set& read_only_variables() const
        { return readonly_vars_; }

public:
    virtual bool is_satisfied() const = 0;

    virtual bool is_in_solver() const
        { return times_added_ != 0; }

    virtual bool is_okay_for_simplex_solver() const
        { return true; }

    virtual bool is_read_only(variable v) const
        { return readonly_vars_.count(v) != 0; }

public:
    void change_strength(const strength& new_strength)
    {
        if (times_added_ == 0)
            strength_ = new_strength;
        else
            throw too_difficult();
    }

    void change_weight(double new_weight)
    {
        if (times_added_ == 0)
            weight_ = new_weight;
        else
            throw too_difficult();
    }

    constraint& add_readonly_vars(const variable_set& vars)
    {
        readonly_vars_.insert(vars.begin(), vars.end());
        return *this;
    }

    symbolic_weight get_symbolic_weight() const
        { return strength_.weight(); }

    double adjusted_symbolic_weight() const
        { return (get_symbolic_weight() * weight()).as_double(); }

    std::string to_string() const
    {
        return std::string("w{") + std::to_string(weight_) + "} refcount{"
           + std::to_string(times_added_) + "} "
           + expression().to_string();
    }

protected:
    void set_strength(const strength& n)
        { strength_ = n; }

    void set_weight(double n)
        { weight_ = n; }

    void added_to(const solver&)
        { ++times_added_; }

    void removed_from(const solver&)
        { --times_added_; }

private:
    strength        strength_;
    variable_set    readonly_vars_;
    double          weight_;
    unsigned short  times_added_;
};

typedef std::shared_ptr<constraint> constraint_ref;

} // namespace rhea

