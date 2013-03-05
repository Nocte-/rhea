//---------------------------------------------------------------------------
/// \file   solver.hpp
/// \brief  Abstract base class for solvers
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
#include "constraint.hpp"
#include "stay_constraint.hpp"
#include "linear_inequality.hpp"
#include "variable.hpp"

namespace rhea {

/** Base class for solvers. \sa simplex_solver */
class solver
{
public:
    solver()
        : auto_solve_(true)
    { }

    virtual ~solver() { }


    virtual solver& solve() { return *this; }

    virtual void resolve() = 0;

    solver& set_autosolve(bool is_auto = true)
    {
        auto_solve_ = true;
        if (auto_solve_)
            solve();

        return *this;
    }

public:
    solver& add_constraint(const constraint& c)
    {
        add_constraint_(c);
        return *this;
    }

    solver& add_constraint(const linear_equation& c,
                           const strength& s = strength::required(),
                           double weight = 1.0)
    {
        return add_constraint(constraint(c, s, weight));
    }

    solver& add_constraint(const linear_inequality& c,
                           const strength& s = strength::required(),
                           double weight = 1.0)
    {
        return add_constraint(constraint(c, s, weight));
    }

    solver& add_constraints(const constraint_list& cs)
    {
        for (auto& c : cs) add_constraint(c);
        return *this;
    }

    solver& add_lower_bound(const variable& v, double lower)
    {
        return add_constraint(v >= lower);
    }

    solver& add_upper_bound(const variable& v, double upper)
    {
        return add_constraint(v <= upper);
    }

    solver& add_bounds(const variable& v, double lower, double upper)
    {
        return add_lower_bound(v, lower).add_upper_bound(v, upper);
    }

    solver& add_stay(const variable& v,
                     const strength& s = strength::weak(),
                     double weight = 1.0)
    {
        add_constraint(std::make_shared<stay_constraint>(v, s, weight));
        return *this;
    }

    solver& add_stays(const variable_set& vs,
                      const strength& s = strength::weak(),
                      double weight = 1.0)
    {
        for (auto& v : vs)
            add_constraint(std::make_shared<stay_constraint>(v, s, weight));

        return *this;
    }

    solver& add_stays(const std::list<variable>& vs,
                      const strength& s = strength::weak(),
                      double weight = 1.0)
    {
        for (auto& v : vs)
            add_constraint(std::make_shared<stay_constraint>(v, s, weight));

        return *this;
    }

    solver& remove_constraint(const constraint& c)
    {
        remove_constraint_(c);
        return *this;
    }

    solver& remove_constraints(const constraint_list& cs)
    {
        for (auto& c : cs) remove_constraint(c);
        return *this;
    }

protected:
    virtual solver& add_constraint_(const constraint& c) = 0;
    virtual solver& remove_constraint_(const constraint& c) = 0;

    bool auto_solve_;
};

} // namespace rhea
