//---------------------------------------------------------------------------
/// \file   linear_inequality.hpp
/// \brief  A linear inequality constraint
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

#include "errors.hpp"
#include "linear_constraint.hpp"
#include "relation.hpp"

namespace rhea {

/** A constraint of the form \f$expr \geq 0\f$. */
class linear_inequality : public linear_constraint
{
public:
    linear_inequality ()
        : linear_constraint(0.0, strength::required(), 1.0)
        , strict_inequality_(false)
    { }

    linear_inequality (linear_expression expr,
                       strength s = strength::required(),
                       double weight = 1.0)
        : linear_constraint(std::move(expr), s, weight)
        , strict_inequality_(false)
    { }

    linear_inequality(const variable& v, relation op, linear_expression expr,
                      strength s = strength::required(),
                      double weight = 1.0)
        : linear_constraint(std::move(expr), s, weight)
        , strict_inequality_(false)
    {
        switch (op.type())
        {
            case relation::geq:
            case relation::gt:
                expr_ *= -1;
                expr_ += v;
                break;

            case relation::leq:
            case relation::lt:
                expr_ -= v;
                break;

            default:
                throw edit_misuse();
        };
        if (op == relation::lt || op == relation::gt)
            strict_inequality_ = true;
    }

    linear_inequality(linear_expression lhs, relation op,
                      linear_expression rhs,
                      strength s = strength::required(),
                      double weight = 1.0)
        : linear_constraint(std::move(rhs), s, weight)
        , strict_inequality_(false)
    {
        switch (op.type())
        {
            case relation::geq:
            case relation::gt:
                expr_ *= -1.0;
                expr_ += lhs;
                break;

            case relation::leq:
            case relation::lt:
                expr_ -= lhs;
                break;

            default:
                throw edit_misuse();
        };
        if (op == relation::lt || op == relation::gt)
            strict_inequality_ = true;
    }

    virtual ~linear_inequality() { }

    virtual bool is_inequality() const
        { return true; }

    virtual bool is_strict_inequality() const
        { return strict_inequality_; }

    virtual bool is_satisfied() const
    {
        double v (expr_.evaluate());
        return strict_inequality_ ? (v > 0) : (v >= 0);
    }

private:
    bool strict_inequality_;
};

//-------------------------------------------------------------------------

inline linear_inequality
operator < (const linear_expression& lhs, const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::lt, rhs);
}

inline linear_inequality
operator <= (const linear_expression& lhs, const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality
operator > (const linear_expression& lhs, const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::gt, rhs);
}

inline linear_inequality
operator >= (const linear_expression& lhs, const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}

//-------------------------------------------------------------------------

inline linear_inequality
operator < (const variable& lhs, const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::lt, rhs);
}

inline linear_inequality
operator <= (const variable& lhs, const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality
operator > (const variable& lhs, const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::gt, rhs);
}

inline linear_inequality
operator >= (const variable& lhs, const linear_expression& rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}

//-------------------------------------------------------------------------

inline linear_inequality
operator < (const variable& lhs, const variable& rhs)
{
    return linear_inequality(lhs, relation::lt, rhs);
}

inline linear_inequality
operator <= (const variable& lhs, const variable& rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality
operator > (const variable& lhs, const variable& rhs)
{
    return linear_inequality(lhs, relation::gt, rhs);
}

inline linear_inequality
operator >= (const variable& lhs, const variable& rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}

//-------------------------------------------------------------------------

inline linear_inequality
operator < (const variable& lhs, double rhs)
{
    return linear_inequality(lhs, relation::lt, rhs);
}

inline linear_inequality
operator <= (const variable& lhs, double rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality
operator > (const variable& lhs, double rhs)
{
    return linear_inequality(lhs, relation::gt, rhs);
}

inline linear_inequality
operator >= (const variable& lhs, double rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}

//-------------------------------------------------------------------------

inline linear_inequality
operator < (const variable& lhs, int rhs)
{
    return linear_inequality(lhs, relation::lt, rhs);
}

inline linear_inequality
operator <= (const variable& lhs, int rhs)
{
    return linear_inequality(lhs, relation::leq, rhs);
}

inline linear_inequality
operator > (const variable& lhs, int rhs)
{
    return linear_inequality(lhs, relation::gt, rhs);
}

inline linear_inequality
operator >= (const variable& lhs, int rhs)
{
    return linear_inequality(lhs, relation::geq, rhs);
}


} // namespace rhea


