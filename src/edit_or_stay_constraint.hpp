//---------------------------------------------------------------------------
/// \file   edit_or_stay_constraint.hpp
/// \brief  Base class for edit- and stay-constraints
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

#include "abstract_constraint.hpp"
#include "linear_expression.hpp"

namespace rhea {

/** Abstract constraint that can be related to a variable, used only as
 ** a base class for edit_constraint and stay_constraint. */
class edit_or_stay_constraint : public abstract_constraint
{
public:
    edit_or_stay_constraint(const variable& v, strength s = strength::required(),
                            double weight = 1.0)
        : abstract_constraint(s, weight)
        , var_(v)
    { }

    virtual ~edit_or_stay_constraint() { }

    const variable& var() const
        { return var_; }

    linear_expression expression() const
        { return linear_expression(var_, -1, var_.value()); }

protected:
    variable var_;
};

} // namespace rhea
