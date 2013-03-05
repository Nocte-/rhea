//---------------------------------------------------------------------------
/// \file   linear_constraint.hpp
/// \brief  Linear constraint
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

/** A constraint based on a linear expression.
 *  Used as a base class for linear_equation and linear_inequality. */
class linear_constraint : public abstract_constraint
{
public:
    linear_constraint(const linear_expression& expr = linear_expression(),
                      const strength& s = strength::required(),
                      double weight = 1.0)
        : abstract_constraint(s, weight)
        , expr_(expr)
    { }

    virtual ~linear_constraint() { }

    linear_expression expression() const
        { return expr_; }

    void change_constant(double c)
        { expr_.set_constant(c); }

protected:
    linear_expression expr_;
};

} // namespace rhea
