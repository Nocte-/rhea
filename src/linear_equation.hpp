//---------------------------------------------------------------------------
/// \file   linear_equation.hpp
/// \brief  A linear equation constraint
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

#include "linear_constraint.hpp"
#include "linear_expression.hpp"

namespace rhea {

class linear_equation : public linear_constraint
{
public:
    linear_equation(linear_expression e, strength s = strength::required(),
                    double weight = 1.0)
        : linear_constraint(std::move(e), s, weight)
    { }

    linear_equation(variable v, linear_expression e,
                    strength s = strength::required(),
                    double weight = 1.0)
        : linear_constraint(std::move(e), s, weight)
    {
        expr_ -= v;
    }

    linear_equation(linear_expression e, variable v,
                    strength s = strength::required(),
                    double weight = 1.0)
        : linear_constraint(std::move(e), s, weight)
    {
        expr_ -= v;
    }

    linear_equation(linear_expression lhs, const linear_expression& rhs,
                    strength s = strength::required(),
                    double weight = 1.0)
        : linear_constraint(std::move(lhs), s, weight)
    {
        expr_ -= rhs;
    }

    virtual ~linear_equation() { }

    virtual bool is_satisfied() const
        { return expr_.evaluate() == 0.0; }
};

} // namespace rhea
