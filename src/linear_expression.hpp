//---------------------------------------------------------------------------
/// \file   linear_expression.hpp
/// \brief  A linear expression
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

#include <unordered_map>
#include "approx.hpp"
#include "variable.hpp"

namespace rhea {

class tableau;

class linear_expression
{
public:
    typedef double t;
    typedef variable_to_number_map::value_type value_type;
    typedef variable_to_number_map::value_type term;

public:
    linear_expression(double num = 0);

    linear_expression(const variable& clv, double value = 1, double constant = 0);

    linear_expression& operator*= (double x);
    linear_expression& operator/= (double x);
    linear_expression& operator*= (const linear_expression& x);
    linear_expression& operator/= (const linear_expression& x);
    linear_expression& operator+= (const linear_expression& x);
    linear_expression& operator-= (const linear_expression& x);
    linear_expression& operator+= (const term& x);
    linear_expression& operator-= (const term& x);

    linear_expression& operator+= (const variable& x)
        { return operator+=(term(x, 1)); }

    linear_expression& operator-= (const variable& x)
        { return operator+=(term(x, -1)); }

    linear_expression& set(const variable& v, double x)
        {
            if (!near_zero(x)) terms_[v] = x;
            return *this;
        }

    // Add expr to this expression from another expression expr.
    // Notify the solver if a variable is added or deleted from this
    // expression.
    linear_expression& add(const linear_expression& expr,
                           const variable& subject, tableau& solver);

    // Add a term c*v to this expression.  If the expression already
    // contains a term involving v, Add c to the existing coefficient.
    // If the new coefficient is approximately 0, delete v.  Notify the
    // solver if v appears or disappears from this expression.
    linear_expression& add(const variable& v, t c,
                           const variable& subject, tableau& solver);

    void erase(const variable& v)
        { terms_.erase(v); }

    // Return a pivotable variable in this expression.  (It is an error
    // if this expression is constant -- signal ExCLInternalError in
    // that case).  Return NULL if no pivotable variables
    variable find_pivotable_variable() const;

    // Replace var with a symbolic expression expr that is equal to it.
    // If a variable has been added to this expression that wasn't there
    // before, or if a variable has been dropped from this expression
    // because it now has a coefficient of 0, inform the solver.
    // PRECONDITIONS:
    //   var occurs with a non-Zero coefficient in this expression.
    void substitute_out(const variable& v, const linear_expression& expr,
                        const variable& subj, tableau& solver);

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
    void change_subject(const variable& old_subj, const variable& new_subj);

    // This linear expression currently represents the equation self=0.
    // Destructively modify it so
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
    //    subject = -c/a - (a1/a)*v1 - ... - (an/a)*vn
    //
    // Note that the term involving subject has been dropped.
    // Returns the reciprocal, so ChangeSubject can use it, too
    t new_subject(const variable& new_subj);

    t evaluate() const;

    t coefficient(variable v) const
    {
        auto i (terms_.find(v));
        return i == terms_.end() ? 0.0 : i->second;
    }

    t constant() const
        { return constant_; }

    void set_constant(t c) { constant_ = c; }

    void increment_constant(t c) { constant_ += c; }

    const variable_to_number_map& terms() const
        { return terms_; }

    bool is_constant() const
        { return terms_.empty(); }

    std::string to_string() const
    {
        std::string result;
        for (auto i (terms_.begin()); i != terms_.end(); ++i)
        {
            result += i->first.name();
            result += " * ";
            result += std::to_string(i->second);
            result += " + ";
        }
        result += std::to_string(constant_);
        return result;
    }

private:
    t constant_;
    variable_to_number_map terms_;
};

//--------------------------------------------------------------------------

inline linear_expression
operator* (linear_expression e, double x)
{
    return e *= x;
}

inline linear_expression
operator* (double x, linear_expression e)
{
    return e *= x;
}

inline linear_expression
operator/ (linear_expression e, double x)
{
    return e /= x;
}

inline linear_expression
operator* (linear_expression e, const linear_expression& x)
{
    return e *= x;
}

inline linear_expression
operator/ (linear_expression e, const linear_expression& x)
{
    return e /= x;
}

inline linear_expression
operator+ (linear_expression e, const linear_expression& x)
{
    return e += x;
}

inline linear_expression
operator- (linear_expression e, const linear_expression& x)
{
    return e -= x;
}

//--------------------------------------------------------------------------

inline linear_expression
operator* (const variable& v, double x)
{
    return linear_expression(v, x);
}

inline linear_expression
operator/ (const variable& v, double x)
{
    return linear_expression(v, 1.0/x);
}

inline linear_expression
operator+ (const variable& v, double x)
{
    return linear_expression(v, 1, x);
}

inline linear_expression
operator- (const variable& v, double x)
{
    return linear_expression(v, -1, x);
}

inline linear_expression
operator+ (const variable& v, const variable& w)
{
    return linear_expression(v) += w;
}

inline linear_expression
operator- (const variable& v, const variable& w)
{
    return linear_expression(v) -= w;
}

} // namespace rhea

