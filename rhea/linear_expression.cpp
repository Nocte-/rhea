//---------------------------------------------------------------------------
//  linear_expression.cpp
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
// Copyright 2012, 2013, nocte@hippie.nu
//---------------------------------------------------------------------------
#include "linear_expression.hpp"

#include <algorithm>
#include <cmath>

#include "approx.hpp"
#include "tableau.hpp"

namespace rhea {

linear_expression::linear_expression(double constant)
    : constant_(constant)
{ }

linear_expression::linear_expression(const variable& v, double mul,
                                     double constant)
    : constant_(constant)
{
    terms_[v] = mul;
}

linear_expression& linear_expression::operator*= (double x)
{
    constant_ *= x;
    for (auto& p : terms_)
        p.second *= x;

    return *this;
}

linear_expression& linear_expression::operator/= (double x)
{
    constant_ /= x;
    for (auto& p : terms_)
        p.second /= x;

    return *this;
}

linear_expression& linear_expression::operator*= (const linear_expression& x)
{
    if (is_constant())
        return *this = x * constant();

    else if (!x.is_constant())
        throw nonlinear_expression();

    return operator*=(x.constant());
}

linear_expression& linear_expression::operator/= (const linear_expression& x)
{
    if (is_constant())
        return *this = x / constant();

    else if (!x.is_constant())
        throw nonlinear_expression();

    return operator/=(x.constant());
}

linear_expression& linear_expression::operator+= (const linear_expression& x)
{
    constant_ += x.constant_;
    for (auto& p : x.terms_)
        operator+=(p);

    return *this;
}

linear_expression& linear_expression::operator+= (const term& x)
{
    auto i (terms_.find(x.first));
    if (i == terms_.end())
    {
        if (!near_zero(x.second))
            terms_[x.first] = x.second;
    }
    else if (near_zero(i->second += x.second))
    {
        terms_.erase(i);
    }
    return *this;
}

linear_expression& linear_expression::operator-= (const linear_expression& x)
{
    constant_ -= x.constant_;
    for (auto& p : x.terms_)
        operator-=(p);

    return *this;
}

linear_expression& linear_expression::operator-= (const term& x)
{
    auto i (terms_.find(x.first));
    if (i == terms_.end() && !near_zero(x.second))
    {
        terms_[x.first] = -x.second;
    }
    else if (near_zero(i->second -= x.second))
    {
        terms_.erase(i);
    }
    return *this;
}

linear_expression&
linear_expression::add(const linear_expression& x, const variable& subject,
                       tableau& solver)
{
    constant_ += x.constant_;
    for (const auto& p : x.terms_)
        add(p.first, p.second, subject, solver);

    return *this;
}

linear_expression&
linear_expression::add(const variable& v, double c, const variable& subject,
                       tableau& solver)
{
    auto i (terms_.find(v));
    if (i == terms_.end())
    {
        if(!near_zero(c))
        {
            terms_[v] = c;
            solver.note_added_variable(v, subject);
        }
    }
    else if (near_zero(i->second += c))
    {
        terms_.erase(i);
        solver.note_removed_variable(v, subject);
    }

    return *this;
}

variable linear_expression::find_pivotable_variable() const
{
    auto found (std::find_if(terms_.begin(), terms_.end(),
                [&](const value_type& x){ return x.first.is_pivotable(); }));

    return found == terms_.end() ? variable::nil_var() : found->first;
}

double linear_expression::evaluate() const
{
    double result (constant_);
    for (const term& p : terms_)
        result += p.first.value() * p.second;

    return result;
}

double linear_expression::new_subject(const variable& subj)
{
    auto i (terms_.find(subj));
    assert(i != terms_.end());
    double reciprocal (1.0 / i->second);
    terms_.erase(i);
    operator*=(-reciprocal);

    return reciprocal;
}

void
linear_expression::change_subject(const variable& old_subj,
                                  const variable& new_subj)
{
    assert(!new_subj.is_nil());
    assert(!near_zero(coefficient(new_subj)));
    if (old_subj.is(new_subj))
        return;

    terms_[old_subj] = new_subject(new_subj);
}

void
linear_expression::substitute_out(const variable& var,
                                  const linear_expression& expr,
                                  const variable& subj, tableau& solver)
{
    auto it (terms_.find(var));
    assert(it != terms_.end());
    double multiplier (it->second);
    terms_.erase(it);

    increment_constant(multiplier * expr.constant());
    for (auto& p : expr.terms())
    {
        const variable& v (p.first);
        double c (multiplier * p.second);

        auto oc (terms_.find(v));
        if (oc != terms_.end())
        {
            double new_coeff (oc->second + c);
            if (near_zero(new_coeff))
            {
                solver.note_removed_variable(oc->first, subj);
                terms_.erase(oc);
            }
            else
            {
                oc->second = new_coeff;
            }
        }
        else
        {
            terms_[v] = c;
            solver.note_added_variable(v, subj);
        }
    }
}

} // namespace rhea


