//---------------------------------------------------------------------------
/// \file   simplex_solver.hpp
/// \brief  Implementation of a solver using a simplex algorithm
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <unordered_map>
#include <vector>

#include "constraint.hpp"
#include "strength.hpp"
#include "symbol.hpp"

namespace rhea
{
class simplex_solver;
}

namespace std
{
ostream& operator<< (ostream&, const rhea::simplex_solver&);
}

namespace rhea
{

using row = expression<symbol>;

/** Solver that implements the Cassowary incremental simplex algorithm. */
class simplex_solver
{    
public:
    /** This struct is used as a parameter for the suggest() function. */
    struct suggestion
    {
        const variable& v;
        double suggested_value;
    };

public:
    simplex_solver();

    constraint add_constraint(const constraint& c);

    void add_constraints(std::initializer_list<constraint> list);

    void set_constant(const constraint& c, double constant);

    void remove_constraint(const constraint& c);

    void remove_constraints(std::initializer_list<constraint> list);

    bool has_constraint(const constraint& c) const;

    /** Add an edit constraint for a given variable.
     * The application should call this for every variable it is planning
     * to suggest a new value for. Edit constraints cannot be required. */
    simplex_solver& add_edit_var(const variable& v,
                                 strength s = strength::strong());

    void add_edit_vars(std::initializer_list<variable> list,
                       strength s = strength::strong());

    void remove_edit_var(const variable& v);

    void remove_edit_vars(std::initializer_list<variable> list);

    bool has_edit_var(const variable& v) const;

    /** Suggest a new value for an edit variable.
     *  The variable needs to be added as an edit variable first. */
    simplex_solver& suggest_value(const variable& v, double x);

    /** Suggest a new value for an edit variable.
     *  This function will register the variable for you if needed,
     *  suggest the value, and call update_variables() afterward. */
    void suggest(const variable& v, double value);

    /** Suggest new values for a set of edit variables.
     *  This function will register the variables for you if needed,
     *  suggest the values, and call update_variables() afterward. */
    void suggest(std::initializer_list<suggestion> list);

    bool has_variable(const variable& v) const;

    /** Write the values from the tableau back to the external
     ** variables.
     * If you're not using auto_update, you'll have to call this function
     * right before you want to start using the results in your variables.
     */
    void update_external_variables();

    void change_strength(const constraint& c, strength new_strength);

    /** Turn auto-updating on or off. */
    void auto_update(bool flag);

    /** Query the auto-update state. */
    bool auto_update() const;

private:
    void add_constraint_(const constraint& c);
    void set_constant_(const constraint& c, double constant);
    void remove_constraint_(const constraint& c);

    /** Keep track of the constraints' marker and slack variables. */
    struct constraint_info
    {
        symbol marker;
        symbol other;
        double prev_constant;
    };

    /** Bundles a constraint, positive and negative error variables, and a
     ** prior edit constant for edit variables. */
    struct edit_info
    {
        bool operator==(const symbol& comp) const { return plus.is(comp); }

        constraint c;
        symbol plus;
        symbol minus;
        double prev_constant;
    };

    /** Internal bookkeeping of the constraint and error variables introduced
     ** by a stay on a variable. */
    struct stay_info
    {
        constraint c;
        symbol plus;
        symbol minus;
    };

    /** The result of make_expression(), a row and its variables.
     * For required constraints, var1 and var2 hold the marker and slack.
     * For non-required ones, they hold the error variables. */
    struct expression_result
    {
        row r;
        symbol var1;
        symbol var2;
    };

private:
    /** Make a new linear expression representing the constraint c,
     ** replacing any basic variables with their defining expressions.
     * Normalize if necessary so that the constant is non-negative.  If
     * the constraint is non-required, give its error variables an
     * appropriate weight in the objective function. */
    expression_result make_expression(const constraint& c);

    /** Add the constraint \f$expr = 0\f$ to the inequality tableau using
     ** an artificial variable.
     * To do this, create an artificial variable \f$a\f$, and add the
     * expression \f$a = expr\f$ to the inequality tableau.
     * Then we try to solve for \f$a = 0\f$, the return value indicates
     * whether this has succeeded or not.
     * @return True if the expression could be added.
     *         False if the constraint could not be satisfied. */
    bool add_with_artificial_variable(const row& r);

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
     * artificial variable and use that variable as the subject &mdash; this is
     * done outside this method though.
     *
     * Note: in checking for variables that are new to the solver, we
     * ignore whether a variable occurs in the objective function, since
     * new slack variables are added to the objective function by
     * make_expression(), which is called before this method.
     *
     * \param expr  The expression that is being added to the solver
     * \return An appropriate subject, or nil */
    symbol choose_subject(const expression_result& expr);

    /** Optimize using the dual of the simplex algorithm. */
    void dual_optimize();

    /** Minimize the value of the given objective.
     * \pre The tableau is feasible.
     * \param z The objective to optimize for */
    void optimize(const row& z);

    void suggest_value_(const variable& v, double value);

    void reset_stay_constants();

    /** Get the symbol for the given variable.
     * If a symbol does not exist for the variable, one will be created. */
    symbol get_var_symbol(const variable& v);

    /** Substitute the parametric symbol with the given row.
     * This method will substitute all instances of the parametric symbol
     * in the tableau and the objective function with the given row. */
    void substitute_out(const symbol& s, const row& r);

    std::unordered_map<symbol, row>::iterator
    get_marker_leaving_row(const symbol& marker);

    void add(row& r, symbol sym, double coeff);

    void autoupdate();

private:
    friend std::ostream& std::operator<< (std::ostream&, const simplex_solver&);

private:
    bool auto_update_;
    std::unordered_map<variable, symbol> vars_;
    std::unordered_map<symbol, row> rows_;
    std::unordered_map<constraint, constraint_info> constraints_;
    std::vector<symbol> infeasible_rows_;

    std::unordered_map<variable, edit_info> edits_;
    std::unordered_map<variable, stay_info> stays_;

    row objective_;
    row artificial_;
};

} // namespace rhea
