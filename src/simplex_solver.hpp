//---------------------------------------------------------------------------
/// \file   simplex_solver.hpp
/// \brief  Implementation of a solver using a simplex algorithm
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

#include <functional>
#include <list>
#include <stack>
#include <vector>

#include "edit_constraint.hpp"
#include "linear_expression.hpp"
#include "linear_inequality.hpp"
#include "solver.hpp"
#include "stay_constraint.hpp"
#include "tableau.hpp"
#include "objective_variable.hpp"
#include "point.hpp"

namespace rhea {

/** Solver that implements the Cassowary simplex algorithm. */
class simplex_solver : public solver, public tableau
{
    typedef tableau super;

public:
    typedef std::function<void(simplex_solver&)>            event_cb;
    typedef std::function<void(const variable&, simplex_solver&)> variable_cb;

    event_cb    on_resolve;
    variable_cb on_variable_change;

    typedef std::unordered_map<constraint, variable_set> constraint_to_varset_map;
    typedef std::unordered_map<constraint, variable>   constraint_to_var_map;
    typedef std::unordered_map<variable, constraint>   var_to_constraint_map;

public:
    simplex_solver()
        : solver()
        , objective_(new objective_variable("z"))
        , needs_solving_ (true)
        , explain_failure_ (false)
    {
        rows_[objective_]; // Create an empty row for the objective
        cedcns_.push(0);
    }

    virtual ~simplex_solver() { }

    simplex_solver& add_lower_bound(const variable& v, double lower)
    {
        return add_constraint(new linear_inequality(v - lower));
    }

    simplex_solver& add_upper_bound(const variable& v, double upper)
    {
        return add_constraint(new linear_inequality(upper - v));
    }

    simplex_solver& add_bounds(const variable& v, double lower, double upper)
    {
        return add_lower_bound(v, lower).add_upper_bound(v, upper);
    }

    simplex_solver& add_constraint(const constraint& c);

    simplex_solver& add_constraint(const linear_equation& c,
                                   const strength& s = strength::required(),
                                   double weight = 1.0)
    {
        return add_constraint(constraint(c, s, weight));
    }

    simplex_solver& add_constraint(const linear_inequality& c,
                                   const strength& s = strength::required(),
                                   double weight = 1.0)
    {
        return add_constraint(constraint(c, s, weight));
    }



    simplex_solver& remove_constraint(const constraint& c);

    simplex_solver& add_edit_var(const variable& v,
                                 const strength& s = strength::strong(),
                                 double weight = 1.0)
    {
        return add_constraint(new edit_constraint(v, s, weight));
    }

    simplex_solver& add_edit_var(const point& p,
                                 const strength& s = strength::strong(),
                                 double weight = 1.0)
    {
        return  add_constraint(new edit_constraint(p.x, s, weight))
               .add_constraint(new edit_constraint(p.y, s, weight));
    }

    simplex_solver& begin_edit();

    simplex_solver& end_edit();

    simplex_solver& remove_edit_var(const variable& v);

    simplex_solver& remove_edit_vars_to(size_t n);

    simplex_solver& remove_all_edit_vars()
    {
        remove_edit_vars_to(0);
        return *this;
    }

    simplex_solver& add_point_stays(const std::vector<point>& points,
                                    const strength& s = strength::weak(),
                                    double weight = 1.0);

    simplex_solver& add_point_stay(const point& v,
                                   const strength& s = strength::weak(),
                                   double weight = 1.0);


    simplex_solver& add_stay(const variable& v,
                             const strength& s = strength::weak(),
                             double weight = 1.0)
    {
        return add_constraint(new stay_constraint(v, s, weight));
    }

    void resolve();

    /** Suggest a new value for an edit variable.
     *  The variable needs to be added as an edit variable,
     *  and begin_edit() needs to be called first.
     *  The tableau will not be solved completely until
     *  after resolve() or end_edit() has been called. */
    simplex_solver& suggest_value(const variable& v, double x);

    /** Suggest a new value for a point variable.
     *  The variable needs to be added as an edit variable,
     *  and begin_edit() needs to be called first.
     *  The tableau will not be solved completely until
     *  after resolve() or end_edit() has been called. */
    simplex_solver& suggest_value(const point& p, double x, double y)
    {
        return suggest_value(p.x, x).suggest_value(p.y, y);
    }

    /** If autosolving has been turned off, client code needs to explicitly
     ** call solve() before accessing variables values. */
    simplex_solver& solve()
    {
        if (needs_solving_)
        {
            optimize(objective_);
            set_external_variables();
        }
        return *this;
    }

    simplex_solver& set_edited_value(variable& v, double n)
    {
        if (contains_variable(v))
        {
            change(v, n);
        }
        else if (!approx(n, v.value()))
        {
            add_edit_var(v);
            begin_edit();
            suggest_value(v, n);
            end_edit();
        }
        return *this;
    }

    bool contains_variable(const variable& v)
    {
        return columns_has_key(v) || is_basic_var(v);
    }

    simplex_solver& add_var(const variable& v)
    {
        if (!contains_variable(v))
            add_stay(v);

        return *this;
    }

    const constraint_to_var_map& constraint_map() const
        { return marker_vars_; }

    const var_to_constraint_map& marker_map() const
        { return constraints_marked_; }

    bool is_constraint_satisfied(const constraint& c) const;


    /** Reset all external variables to their current values.
     * Note: this triggers all callbacks, which might be used to copy the
     * variable's value to another variable. */
    void update_external_variables()
        { set_external_variables(); }

    void change_strength_and_weight(constraint c, const strength& s, double weight);
    void change_strength(constraint c, const strength& s);
    void change_weight(constraint c, double weight);

    /** Reset all stay constraint constants.
     * Each of the non-required stays will be represented by the equation
     * \f$v = v' + e_{plus} - e_{minus}\f$, where \f$v\f$ is the variable
     * associated with the stay, \f$v'\f$ is the previous value of
     * \f$v\f$, and \f$e_{plus}\f$ and \f$e_{minus}\f$ are slack variables
     * that hold the error for satisfying the constraint.
     *
     * We are about to change something, and we want to fix the constants
     * in the equations representing the stays.  If both \f$e_{plus}\f$
     * and \f$e_{minus}\f$ are nonbasic, they are zero in the current
     * solution, meaning the previous stay was exactly satisfied.  In this
     * case nothing needs to be changed.  Otherwise one of them is basic,
     * and the other must occur only in the expression for that basic error
     * variable.  In that case, the constant in the expression is set to
     * zero. */
    void reset_stay_constants();

    simplex_solver& set_auto_reset_stay_constants(bool f = true)
    {
        auto_reset_stay_constants_ = f;
        if (f)
            reset_stay_constants();

        return *this;
    }

    bool is_auto_reset_stay_constants() const
        { return auto_reset_stay_constants_; }

protected:
    /** This is a privately-used struct that bundles a constraint, its
     ** positive and negative error variables, and its prior edit constant.
     */
    struct edit_info
    {
        edit_info(const variable& v_, constraint c_,
                  variable plus_, variable minus_, double prev_constant_)
            : v(v_), c(c_), plus(plus_), minus(minus_), prev_constant(prev_constant_)
        { }

        bool operator== (const variable& comp) const
            { return v.is(comp); }

        variable v;
        constraint c;
        variable plus;
        variable minus;
        double prev_constant;
    };

    /** Bundles an expression, a plus and minus slack variable, and a
     ** prior edit constant.
     *  This struct is only used as a return variable of make_epression().*/
    struct expression_result
    {
        linear_expression expr;
        variable minus;
        variable plus;
        double   previous_constant;

        expression_result()
            : minus(variable::nil())
            , plus(variable::nil())
            , previous_constant(0.0)
        { }
    };

    /** Make a new linear expression representing the constraint c,
     ** replacing any basic variables with their defining expressions.
     * Normalize if necessary so that the constant is non-negative.  If
     * the constraint is non-required, give its error variables an
     * appropriate weight in the objective function. */
    expression_result make_expression(const constraint& c);

    /** Add the constraint \f$expr = 0\f$ to the inequality tableau using
     ** an artificial variable.
     * To do this, create an artificial variable \f$a_0\f$, and add the
     * expression \f$a_0 = expr\f$ to the inequality tableau.
     * Then we try to solve for \f$a_0 = 0\f$, the return value indicates
     * whether this has succeeded or not.
     * @return True iff the expression could be added */
    bool add_with_artificial_variable(linear_expression& expr);

    /** Add the constraint \f$expr = 0\f$ to the inequality tableau.
     * @return True iff the expression could be added */
    bool try_adding_directly(linear_expression& expr);

    /** Try to choose a subject (that is, a variable to become basic) from
     ** among the current variables in \a expr.
     * If expr contains any unrestricted variables, then we must choose an
     * unrestricted variable as the subject.  Also, if the subject is new to
     * the solver, we won't have to do any substitutions, so we prefer new
     * variables to ones that are currently noted as parametric.
     *
     * If expr contains only restricted variables, if there is a restricted
     * variable with a negative coefficient that is new to the solver we can
     * make that the subject.  Otherwise we return nil, and have to add an
     * artificial variable and use that variable as the subject -- this is
     * done outside this method though.
     *
     * Note: in checking for variables that are new to the solver, we
     * ignore whether a variable occurs in the objective function, since
     * new slack variables are added to the objective function by
     * make_expression(), which is called before this method.
     *
     * \param expr  The expression that is being added to the solver
     * \return An appropriate subject, or nil */
    variable choose_subject(linear_expression& expr);

    void delta_edit_constant(double delta, const variable& v1, const variable& v2);

    /** Optimize using the dual algorithm. */
    void dual_optimize();

    /** Minimize the value of an objective.
     * \pre The tableau is feasible.
     * \param z The objective to optimize for */
    void optimize(const variable& z);

    /** Perform a pivot operation.
     *  Move entry into the basis (i.e. make it a basic variable), and move
     *  exit out of the basis (i.e., make it a parametric variable).
     */
    void pivot(const variable& entry, const variable& exit);

    /** Set the external variables known to this solver to their appropriate
     ** values.
     * Set each external basic variable to its value, and set each
     * external parametric variable to zero.  Variables that are internal
     * to the solver don't actually store values &mdash; their
     * values are just implicit in the tableu &mdash; so we don't need to
     * set them. */
    void set_external_variables();

    simplex_solver& remove_constraint_internal(const constraint& c);

    void change(variable& v, double n)
    {
        v.change_value(n);
        if (on_variable_change)
            on_variable_change(v, *this);
    }

private:
    // the arrays of positive and negative error vars for the stay constraints
    // (need both positive and negative since they have only non-negative values)
    std::vector<variable> stay_minus_error_vars_;
    std::vector<variable> stay_plus_error_vars_;

    constraint_to_varset_map    error_vars_;
    constraint_to_var_map       marker_vars_;
    var_to_constraint_map       constraints_marked_;

    variable    objective_;

    // Map edit variables to their constraints, errors, and prior
    // values
    std::list<edit_info>  edit_info_list_;

    bool        auto_reset_stay_constants_;
    bool        needs_solving_;
    bool        explain_failure_;

    std::stack<size_t> cedcns_;
};

/** Scoped edit action.
 * This class calls begin_edit() on a simplex_solver upon construction,
 * and end_edit() as it goes out of scope.  This can be used as an
 * alternative to calling these two functions manually.
 *
 * \code

variable x(4), y(6);
simplex_solver solv;

solv.add_edit_variable(x).add_edit_variable(y);
{
scoped_edit user_input(solv);
solv.suggest_value(x, 2)
    .suggest_value(y, 7);
}
// 'user_input' goes out of scope here and calls solv.end_edit()
 * \endcode */
class scoped_edit
{
public:
    scoped_edit(simplex_solver& s) : s_(s.begin_edit()) { }
    ~scoped_edit() { s_.end_edit(); }

private:
    simplex_solver& s_;
};


} // namespace rhea
