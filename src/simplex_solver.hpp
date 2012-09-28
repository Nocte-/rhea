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

#include "edit_constraint.hpp"
#include "linear_expression.hpp"
#include "linear_inequality.hpp"
#include "solver.hpp"
#include "stay_constraint.hpp"
#include "tableau.hpp"
#include "objective_variable.hpp"
#include "point.hpp"

namespace rhea {

class simplex_solver : public solver, public tableau
{
    typedef tableau super;

public:
    typedef std::function<void(simplex_solver&)>            event_cb;
    typedef std::function<void(const variable&, simplex_solver&)> variable_cb;

    event_cb    on_resolve;
    variable_cb on_variable_change;

    typedef std::unordered_map<constraint_ref, variable_set> constraint_to_varset_map;
    typedef std::unordered_map<constraint_ref, variable>   constraint_to_var_map;
    typedef std::unordered_map<variable, constraint_ref>   var_to_constraint_map;

public:
    simplex_solver()
        : solver()
        , objective_(new objective_variable("z"))
        , epsilon_ (1e-8)
        , needs_solving_ (true)
        , explain_failure_ (false)
    {
        rows_[objective_];
        cedcns_.push(0);
    }

    virtual ~simplex_solver() { }

    simplex_solver& add_lower_bound(const variable& v, double lower)
    {
        return add_constraint(new linear_inequality(linear_expression(v - lower)));
    }

    simplex_solver& add_upper_bound(const variable& v, double upper)
    {
        return add_constraint(new linear_inequality(linear_expression(upper - v)));
    }

    simplex_solver& add_bounds(const variable& v, double lower, double upper)
    {
        return add_lower_bound(v, lower).add_upper_bound(v, upper);
    }

    simplex_solver& add_constraint(constraint* ptr)
    {
        return add_constraint(constraint_ref(ptr));
    }

    simplex_solver& add_constraint(constraint_ref c);

    simplex_solver& remove_constraint(constraint_ref c);

    simplex_solver& add_edit_var(const variable& v,
                                 const strength& s = strength::strong(),
                                 double weight = 1.0)
    {
        return add_constraint(new edit_constraint(v, s, weight));
    }

    simplex_solver& remove_edit_var(variable v);

    simplex_solver& begin_edit()
    {
        if (edit_info_list_.empty())
            throw edit_misuse("begin_edit called with no edit variables");

        infeasible_rows_.clear();
        reset_stay_constants();
        cedcns_.push(edit_info_list_.size());
        return *this;
    }

    simplex_solver& end_edit()
    {
        if (edit_info_list_.empty())
            throw edit_misuse("end_edit called with no edit variables");

        resolve();
        cedcns_.pop();
        remove_edit_vars_to(cedcns_.top());
        return *this;
    }

    simplex_solver& remove_all_edit_vars()
    {
        remove_edit_vars_to(0);
        return *this;
    }

    simplex_solver& remove_edit_vars_to(size_t n);

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

    // Suggest a new value for an edit variable
    // the variable needs to be added as an edit variable
    // and BeginEdit() needs to be called before this is called.
    // The tableau will not be solved completely until
    // after Resolve() has been called
    simplex_solver& suggest_value(variable v, double x);

    // If autosolving has been turned off, client code needs
    // to explicitly call solve() before accessing variables
    // values
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

    bool is_constraint_satisfied(constraint_ref c) const;


    // re-set all the external variables to their current values
    // most importantly, this re-calls all the ChangeClv callbacks
    // (which might be used to copy the ClVariable's value to another
    // variable)
    void update_external_variables()
        { set_external_variables(); }

    void change_strength_and_weight(constraint_ref c, const strength& s, double weight);
    void change_strength(constraint_ref c, const strength& s);
    void change_weight(constraint_ref c, double weight);

    // Each of the non-required stays will be represented by an equation
    // of the form
    //     v = c + eplus - eminus
    // where v is the variable with the stay, c is the previous value of
    // v, and eplus and eminus are slack variables that hold the error
    // in satisfying the stay constraint.  We are about to change
    // something, and we want to fix the constants in the equations
    // representing the stays.  If both eplus and eminus are nonbasic
    // they have value 0 in the current solution, meaning the previous
    // stay was exactly satisfied.  In this case nothing needs to be
    // changed.  Otherwise one of them is basic, and the other must
    // occur only in the Expression for that basic error variable.
    // Reset the Constant in this Expression to 0.
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
    // ClEditInfo is a privately-used class
    // that just wraps a constraint, its positive and negative
    // error variables, and its prior edit Constant.
    // It is used as values in _editInfoList, and replaces
    // the parallel vectors of error variables and previous edit
    // constants from the Smalltalk version of the code.
    struct edit_info
    {
        edit_info(const variable& v_, constraint_ref c_,
                  variable plus_, variable minus_, double prev_constant_)
            : v(v_), c(c_), plus(plus_), minus(minus_), prev_constant(prev_constant_)
        { }

        bool operator== (variable comp) const { return v == comp; }

        variable v;
        constraint_ref c;
        variable plus;
        variable minus;
        double prev_constant;
    };

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

    // Make a new linear Expression representing the constraint cn,
    // replacing any basic variables with their defining expressions.
    // Normalize if necessary so that the Constant is non-negative.  If
    // the constraint is non-required give its error variables an
    // appropriate weight in the objective function.
    expression_result make_expression(constraint_ref c);


    // Add the constraint expr=0 to the inequality tableau using an
    // artificial variable.  To do this, create an artificial variable
    // av and Add av=expr to the inequality tableau, then make av be 0.
    // (Raise an exception if we can't attain av=0.)
    // (Raise an exception if we can't attain av=0.) If the Add fails,
    // prepare an explanation in e that describes why it failed (note
    // that an empty explanation is considered to mean the explanation
    // encompasses all active constraints.
    bool add_with_artificial_variable(linear_expression& expr);


    // We are trying to Add the constraint expr=0 to the appropriate
    // tableau.  Try to Add expr directly to the tableax without
    // creating an artificial variable.  Return true if successful and
    // false if not.
    bool try_adding_directly(linear_expression& expr);

    // We are trying to Add the constraint expr=0 to the tableaux.  Try
    // to choose a subject (a variable to become basic) from among the
    // current variables in expr.  If expr contains any unrestricted
    // variables, then we must choose an unrestricted variable as the
    // subject.  Also, if the subject is new to the solver we won't have
    // to do any substitutions, so we prefer new variables to ones that
    // are currently noted as parametric.  If expr contains only
    // restricted variables, if there is a restricted variable with a
    // negative coefficient that is new to the solver we can make that
    // the subject.  Otherwise we can't find a subject, so return nil.
    // (In this last case we have to Add an artificial variable and use
    // that variable as the subject -- this is done outside this method
    // though.)
    //
    // Note: in checking for variables that are new to the solver, we
    // ignore whether a variable occurs in the objective function, since
    // new slack variables are added to the objective function by
    // 'NewExpression:', which is called before this method.
    variable choose_subject(linear_expression& expr);

    // Each of the non-required edits will be represented by an equation
    // of the form
    //    v = c + eplus - eminus
    // where v is the variable with the edit, c is the previous edit
    // value, and eplus and eminus are slack variables that hold the
    // error in satisfying the edit constraint.  We are about to change
    // something, and we want to fix the constants in the equations
    // representing the edit constraints.  If one of eplus and eminus is
    // basic, the other must occur only in the Expression for that basic
    // error variable.  (They can't both be basic.)  Fix the Constant in
    // this Expression.  Otherwise they are both nonbasic.  Find all of
    // the expressions in which they occur, and fix the constants in
    // those.  See the UIST paper for details.
    // (This comment was for resetEditConstants(), but that is now
    // gone since it was part of the screwey vector-based interface
    // to resolveing. --02/15/99 gjb)
    void delta_edit_constant(double delta, variable v1, variable v2);

    // We have set new values for the constants in the edit constraints.
    // Re-Optimize using the dual simplex algorithm.
    void dual_optimize();

    // Minimize the value of the objective.  (The tableau should already
    // be feasible.)
    void optimize(variable z);

    // Do a Pivot.  Move entryVar into the basis (i.e. make it a basic variable),
    // and move exitVar out of the basis (i.e., make it a parametric variable)
    void pivot(variable entryVar, variable exitVar);

    // Set the external variables known to this solver to their appropriate values.
    // Set each external basic variable to its value, and set each
    // external parametric variable to 0.  (It isn't clear that we will
    // ever have external parametric variables -- every external
    // variable should either have a stay on it, or have an equation
    // that defines it in terms of other external variables that do have
    // stays.  For the moment I'll put this in though.)  Variables that
    // are internal to the solver don't actually store values -- their
    // values are just implicit in the tableu -- so we don't need to set
    // them.
    void set_external_variables();

    // this gets called by RemoveConstraint and by AddConstraint when the
    // contraint we're trying to Add is inconsistent
    simplex_solver& remove_constraint_internal(constraint_ref c);

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

    variable  objective_;

    // Map edit variables to their constraints, errors, and prior
    // values
    std::list<edit_info>  edit_info_list_;

    const double epsilon_;

    bool        auto_reset_stay_constants_;
    bool        needs_solving_;
    bool        explain_failure_;

    std::stack<size_t> cedcns_;
};

} // namespace rhea
