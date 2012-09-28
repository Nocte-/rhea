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
#include "variable.hpp"

namespace rhea {

class solver
{
public:
    solver()
        : auto_solve_(true)
    { }

    virtual ~solver() { }

    virtual solver& add_constraint(const constraint_ref& c) = 0;
    virtual solver& remove_constraint(const constraint_ref& c) = 0;

    virtual solver& solve() { return *this; }

    virtual void resolve() = 0;

    solver& set_autosolve(bool is_auto = true)
    {
        auto_solve_ = true;
        if (auto_solve_)
            solve();

        return *this;
    }

protected:
    bool auto_solve_;
};

} // namespace rhea
