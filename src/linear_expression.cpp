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
// Copyright 2012, nocte@hippie.nu
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

linear_expression::linear_expression(const variable& v, t value, t constant)
    : constant_(constant)
{
    terms_[v] = value;
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
    if (i == terms_.end() && !near_zero(x.second))
    {
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
linear_expression::add(const variable& v, t c, const variable& subject,
                       tableau& solver)
{
    auto i (terms_.find(v));
    if (i == terms_.end() && !near_zero(c))
    {
        terms_[v] = c;
        solver.note_added_variable(v, subject);
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

    return found == terms_.end() ? variable::nil() : found->first;
}

double linear_expression::evaluate() const
{
    double result (constant_);
    for (const term& p : terms_)
        result += p.first.value() * p.second;

    return result;
}

// This linear expression currently represents the equation self=0.  Destructively modify it so
// that subject=self represents an equivalent equation.
//
// Precondition: subject must be one of the variables in this expression.
// NOTES
//   Suppose this expression is
//     c + a*subject + a1*v1 + ... + an*vn
//   representing
//     c + a*subject + a1*v1 + ... + an*vn = 0
// The modified expression will be
//    subject = -c/a - (a1/a)*v1 - ... - (an/a)*vn
//   representing
//    subject = -c/a - (a1/a)*v1 - ... - (an/a)*vn = 0
//
// Note that the term involving subject has been dropped.
//
// Returns the reciprocal, so that NewSubject can be used by ChangeSubject
double linear_expression::new_subject(variable subj)
{
    auto i (terms_.find(subj));
    assert(i != terms_.end());
    double reciprocal (1.0 / i->second);
    terms_.erase(i);
    operator*=(-reciprocal);

    return reciprocal;
}

// This linear expression currently represents the equation
// oldSubject=self.  Destructively modify it so that it represents
// the equation NewSubject=self.
//
// Precondition: NewSubject currently has a nonzero coefficient in
// this expression.
//
// NOTES
//   Suppose this expression is c + a*NewSubject + a1*v1 + ... + an*vn.
//
//   Then the current equation is
//       oldSubject = c + a*NewSubject + a1*v1 + ... + an*vn.
//   The new equation will be
//        NewSubject = -c/a + oldSubject/a - (a1/a)*v1 - ... - (an/a)*vn.
//   Note that the term involving NewSubject has been dropped.
//
// Basically, we consider the expression to be an equation with oldSubject
// equal to the expression, then Resolve the equation for NewSubject,
// and destructively make the expression what NewSubject is then equal to
void
linear_expression::change_subject(variable old_subj, variable new_subj)
{
    assert(!new_subj.is_nil());
    assert(!near_zero(coefficient(new_subj)));
    if (old_subj == new_subj)
        return;

    terms_[old_subj] = new_subject(new_subj);
}

// Replace var with a symbolic expression expr that is equal to it.
// If a variable has been added to this expression that wasn't there
// before, or if a variable has been dropped from this expression
// because it now has a coefficient of 0, inform the solver.
// PRECONDITIONS:
//   var occurs with a non-Zero coefficient in this expression.
void
linear_expression::substitute_out(variable var, const linear_expression& expr,
                                  variable subj, tableau& solver)
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


