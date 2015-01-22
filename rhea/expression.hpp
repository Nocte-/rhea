//---------------------------------------------------------------------------
/// \file   expression.hpp
/// \brief  An expression of the form \f$a_0x_0 + a_1x_1 + \ldots + c\f$
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include "approx.hpp"
#include "errors.hpp"

namespace rhea
{

/** Linear expression.
 * Expressions have the form \f$a_0x_0 + a_1x_1 + \ldots + c\f$, where \f$x_n\f$
 * is a variable, \f$a_n\f$ are non-zero coefficients, and \f$c\f$ is a
 * constant.
 */
template <typename T>
class expression
{
public:
    typedef T var_type;
    typedef std::unordered_map<T, double> terms_map;
    typedef std::pair<T, double> term;

public:
    expression(double constant = 0.0)
        : constant_{constant}
    {
    }

    expression(const var_type& v, double coeff = 1.0, double constant = 0.0)
        : constant_{constant}
    //, terms_{{v, coeff}}
    {
        terms_[v] = coeff;
    }

    double add(double c) { return constant_ += c; }

    void add(const var_type& v, double coeff = 1.0)
    {
        if (near_zero(terms_[v] += coeff))
            terms_.erase(v);
    }

    /** Erase a variable from the expression. */
    void erase(const var_type& v) { terms_.erase(v); }

    /** Replace \a v with an equivalent symbolic expression.
    * \param v     The variable to be replaced
    * \param expr  The expression to replace it with
    * \return True if \a v was substituted, false if \a v was not part of
    *         the expression.
    */
    bool substitute_out(const var_type& v, const expression& expr)
    {
        auto it = terms_.find(v);
        if (it == terms_.end())
            return false;

        double multiplier = it->second;
        terms_.erase(it);

        // if (near_zero(multiplier))
        //    return true;

        constant_ += multiplier * expr.constant();
        for (auto& p : expr.terms())
            add(p.first, multiplier * p.second);

        return true;
    }

    /** This linear expression currently represents the equation
    ** oldSubject=self, destructively modify it so that it represents
    ** the equation NewSubject=self.
    * Suppose this expression is \f$av_{new} + bv_0 + \ldots + c\f$.
    * Then, if the current equation is:
    * \f[v_{old} = av_{new} + bv_0 + \ldots + c\f]
    * The new equation will become:
    * \f[v_{new} = -\frac{1}{a}v_{old} - \frac{b}{a}v_0 - \ldots -
    *\frac{c}{a}\f]
    *
    * \pre \a new_subj has a nonzero coefficient in this expression.
    */
    void change_subject(const var_type& old_subj, const var_type& new_subj)
    {
        if (std::equal_to<var_type>()(old_subj, new_subj))
            return;

        double tmp = new_subject(new_subj);
        terms_[old_subj] = tmp;
    }

    void solve_for(const var_type& v)
    {
        auto found = terms_.find(v);
        if (found == terms_.end())
            throw std::runtime_error("cannot solve for unknown term");

        auto coeff = -1.0 / found->second;
        terms_.erase(found);
        operator*=(coeff);
    }

    void solve_for(const var_type& lhs, const var_type& rhs)
    {
        operator-=(lhs);
        solve_for(rhs);
    }

    /** Get the coefficient of one of the terms.
    * \param v  The variable to look for
    * \return The term's coefficient, or zero if \a v does not occur
    *         in this expression */
    double coefficient(const var_type& v) const
    {
        auto i = terms_.find(v);
        return i == terms_.end() ? 0.0 : i->second;
    }

    /** Get the constant \f$c\f$ of the expression. */
    double constant() const { return constant_; }

    /** Set the constant \f$c\f$ to a new value. */
    void set_constant(double c) { constant_ = c; }

    /** Get a map of all terms and their coefficients. */
    const terms_map& terms() const { return terms_; }

    /** Returns true iff this expression is constant. */
    bool is_constant() const { return terms_.empty(); }

    /** Returns true iff there are no terms and the constant is zero. */
    bool empty() const { return is_constant() && constant_ == 0; }

public:
    expression& operator*=(double x)
    {
        constant_ *= x;
        for (auto& p : terms_)
            p.second *= x;

        return *this;
    }

    expression& operator/=(double x) { return operator*=(1.0 / x); }

    expression& operator*=(const expression& x)
    {
        if (!is_constant() || !x.is_constant())
            throw nonlinear_expression{};

        return operator*=(x.constant());
    }

    expression& operator/=(const expression& x)
    {
        if (!x.is_constant())
            throw nonlinear_expression{};

        return operator/=(x.constant());
    }

    expression& operator+=(const expression& x)
    {
        constant_ += x.constant_;
        for (auto& p : x.terms_)
            operator+=(p);

        return *this;
    }

    expression& operator-=(const expression& x)
    {
        constant_ -= x.constant_;
        for (auto& p : x.terms_)
            operator-=(p);

        return *this;
    }

    expression& operator+=(const term& x)
    {
        if (near_zero(terms_[x.first] += x.second))
            terms_.erase(x.first);

        return *this;
    }

    expression& operator-=(const term& x)
    {
        if (near_zero(terms_[x.first] -= x.second))
            terms_.erase(x.first);

        return *this;
    }

    expression& operator+=(const var_type& x)
    {
        return operator+=(term(x, 1.0));
    }

    expression& operator-=(const var_type& x)
    {
        return operator-=(term(x, 1.0));
    }

    expression& operator+=(double x)
    {
        constant_ += x;
        return *this;
    }

    expression& operator-=(double x)
    {
        constant_ -= x;
        return *this;
    }

private:
    /** The expression's constant term. */
    double constant_;
    /** A map of all variables and their coefficients. */
    terms_map terms_;
};

} // namespace rhea
