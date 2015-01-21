//---------------------------------------------------------------------------
// simplex_solver.cpp
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#include "simplex_solver.hpp"

#include <algorithm>
#include <iostream>

#include "errors.hpp"

#include "iostream.hpp"
namespace rhea
{
namespace
{

bool all_dummies(const row& r)
{
    return std::all_of(r.terms().begin(), r.terms().end(),
                       [](const row::term& p) { return p.first.is_dummy(); });
}

symbol pivotable_symbol(const row& r)
{
    for (auto& cell : r.terms()) {
        if (cell.first.is_pivotable())
            return cell.first;
    }
    return {};
}

inline row operator*(row e, double x)
{
    return e *= x;
}

inline row operator*(double x, row e)
{
    return e *= x;
}

inline row operator/(row e, double x)
{
    return e /= x;
}

inline row operator*(row e, const row& x)
{
    return e *= x;
}

inline row operator/(row e, const row& x)
{
    return e /= x;
}

inline row operator+(row e, const row& x)
{
    return e += x;
}

inline row operator-(row e, const row& x)
{
    return e -= x;
}

} // anonymous namespace

//---------------------------------------------------------------------------

simplex_solver::simplex_solver()
    : auto_update_{true}
{
}

simplex_solver::expression_result
simplex_solver::make_expression(const constraint& c)
{
    expression_result result;

    auto& r = result.r;
    auto& cexpr = c.expr();
    r.set_constant(cexpr.constant());

    for (const auto& term : cexpr.terms())
        add(r, get_var_symbol(term.first), term.second);

    if (c.is_inequality()) {
        // 'c' is an inequality, so add a slack variable.  The original
        // constraint is expr>=0, so that the resulting equality is
        // expr-slackVar==0.  If 'c' is also non-required, add a negative
        // error variable, giving:
        //    expr - slackVar = -errorVar
        //    expr - slackVar + errorVar = 0.
        // Since both of these variables are newly created we can just add
        // them to the expression (they can't be basic).
        double coeff = c.oper() == relation::leq ? 1.0 : -1.0;
        auto slack = symbol::slack();
        result.var1 = slack;
        r += slack * coeff;

        if (!c.is_required()) {
            auto eminus = symbol::error();
            result.var2 = eminus;
            r -= eminus * coeff;
            objective_.add(eminus, c.get_strength());
        }
    } else if (c.is_required()) {
        // Add a dummy variable to the expression to serve as a marker
        // for this constraint.  The dummy variable is never allowed to
        // enter the basis when pivoting.
        auto dummy = symbol::dummy();
        result.var1 = dummy;
        r += dummy;
    } else {
        // 'c' is a non-required equality.  Add a positive and a negative
        // error variable, making the resulting constraint
        // expr = eplus - eminus (in other words: expr-eplus+eminus=0)
        auto eplus = symbol::error();
        auto eminus = symbol::error();
        result.var1 = eplus;
        result.var2 = eminus;
        r -= eplus;
        r += eminus;
        objective_.add(eplus, c.get_strength());
        objective_.add(eminus, c.get_strength());
    }

    // The constant in the expression should be non-negative.
    // If necessary normalize the Expression by reversing the sign.
    if (r.constant() < 0.0)
        r *= -1.0;

    return result;
}

void simplex_solver::add_constraint_(const constraint& c)
{
    if (has_constraint(c))
        throw duplicate_constraint();

    auto expr = make_expression(c);
    auto subject = choose_subject(expr);

    if (subject.is_nil() && all_dummies(expr.r)) {
        if (!near_zero(expr.r.constant()))
            throw required_failure();

        subject = expr.var1;
    }

    if (subject.is_nil()) {
        if (!add_with_artificial_variable(expr.r))
            throw required_failure();
    } else {
        expr.r.solve_for(subject);
        substitute_out(subject, expr.r);
        rows_[subject] = std::move(expr.r);
    }
    constraints_[c] = constraint_info{expr.var1, expr.var2, -c.expr().constant()};
    optimize(objective_);
}

constraint simplex_solver::add_constraint(const constraint& c)
{
    add_constraint_(c);
    autoupdate();
    return c;
}

void simplex_solver::add_constraints(std::initializer_list<constraint> list)
{
    for (auto&& c : list)
        add_constraint_(c);

    autoupdate();
}

void simplex_solver::set_constant(const constraint& c, double constant)
{
    set_constant_(c, constant);
    dual_optimize();
    autoupdate();
}

void simplex_solver::set_constant_(const constraint& c, double constant)
{
    auto found = constraints_.find(c);
    if (found == constraints_.end())
        throw constraint_not_found();

    auto& evs = found->second;
    auto delta = -(constant - evs.prev_constant);
    evs.prev_constant = constant;

    if (evs.marker.is_slack() || c.is_required()) {
        if (c.oper().type() == relation::geq)
            delta = -delta;

        for (auto& r : rows_) {
            auto& expr = r.second;
            expr.add(expr.coefficient(evs.marker) * delta);
            if (!r.first.is_external() && expr.constant() < 0.0)
                infeasible_rows_.emplace_back(r.first);
        }
    } else {
        // Check if the error variables are basic.
        auto row_it = rows_.find(evs.marker);
        if (row_it != rows_.end()) {
            if (row_it->second.add(-delta) < 0.0)
                infeasible_rows_.emplace_back(row_it->first);

            return;
        }

        row_it = rows_.find(evs.other);
        if (row_it != rows_.end()) {
            if (row_it->second.add(delta) < 0.0)
                infeasible_rows_.emplace_back(row_it->first);

            return;
        }

        // Neither is basic.  So they must both be nonbasic, and will both
        // occur in exactly the same expressions.  Find all the expressions
        // in which they occur by finding the column for the minusErrorVar
        // (it doesn't matter whether we look for that one or for
        // plusErrorVar).  Fix the constants in these expressions.
        for (auto& r : rows_) {
            auto& expr = r.second;
            expr.add(expr.coefficient(evs.other) * delta);
            if (!r.first.is_external() && expr.constant() < 0.0)
                infeasible_rows_.emplace_back(r.first);
        }
    }
}

void simplex_solver::remove_constraint_(const constraint& c)
{
    auto found = constraints_.find(c);
    if (found == constraints_.end())
        throw constraint_not_found();

    auto info = found->second;
    constraints_.erase(found);

    // Remove the error effects from the objective function
    // *before* pivoting, or substitutions into the objective
    // will lead to incorrect solver results.
    if (info.marker.is_error())
        add(objective_, info.marker, -c.get_strength());

    if (info.other.is_error())
        add(objective_, info.other, -c.get_strength());

    // If the marker is basic, simply drop the row. Otherwise,
    // pivot the marker into the basis and then drop the row.
    auto row_it = rows_.find(info.marker);
    if (row_it != rows_.end()) {
        rows_.erase(row_it);
    } else {
        row_it = get_marker_leaving_row(info.marker);
        if (row_it == rows_.end())
            throw internal_error("failed to find leaving row");

        symbol leaving = row_it->first;
        row tmp = std::move(row_it->second);
        rows_.erase(row_it);
        tmp.solve_for(leaving, info.marker);
        substitute_out(info.marker, tmp);
    }
    optimize(objective_);
}

void simplex_solver::remove_constraint(const constraint& c)
{
    remove_constraint_(c);
    autoupdate();
}

void simplex_solver::remove_constraints(std::initializer_list<constraint> list)
{
    for (auto&& c : list)
        remove_constraint_(c);

    autoupdate();
}

bool simplex_solver::has_constraint(const constraint& c) const
{
    return constraints_.count(c) != 0;
}

void simplex_solver::suggest_value_(const variable& v, double value)
{
    auto found = edits_.find(v);
    if (found == edits_.end())
        throw unknown_edit_variable();

    auto& info = found->second;
    auto delta = value - info.prev_constant;
    info.prev_constant = value;

    // Check if the error variables are basic.
    auto row_it = rows_.find(info.plus);
    if (row_it != rows_.end()) {
        if (row_it->second.add(-delta) < 0.0)
            infeasible_rows_.emplace_back(row_it->first);

        dual_optimize();
        return;
    }

    row_it = rows_.find(info.minus);
    if (row_it != rows_.end()) {
        if (row_it->second.add(delta) < 0.0)
            infeasible_rows_.emplace_back(row_it->first);

        dual_optimize();
        return;
    }

    // Neither is basic.  So they must both be nonbasic, and will both
    // occur in exactly the same expressions.  Find all the expressions
    // in which they occur by finding the column for the plusErrorVar
    // (it doesn't matter whether we look for that one or for
    // minusErrorVar).  Fix the constants in these expressions.
    for (auto& r : rows_) {
        auto& expr = r.second;
        expr.add(expr.coefficient(info.plus) * delta);
        if (!r.first.is_external() && expr.constant() < 0.0)
            infeasible_rows_.emplace_back(r.first);
    }
}

simplex_solver& simplex_solver::suggest_value(const variable& v, double x)
{
    suggest_value_(v, x);
    dual_optimize();
    return *this;
}

void simplex_solver::suggest(const variable& v, double value)
{
    if (!has_edit_var(v))
        add_edit_var(v);

    suggest_value(v, value);
    autoupdate();
}

void simplex_solver::suggest(std::initializer_list<suggestion> list)
{
    for (auto&& s : list) {
        if (!has_edit_var(s.v))
            add_edit_var(s.v);

        suggest_value_(s.v, s.suggested_value);
    }
    dual_optimize();
    autoupdate();
}

bool simplex_solver::add_with_artificial_variable(const row& r)
{
    // The artificial objective is av, which we know is equal to expr
    // (which contains only parametric variables).
    auto av = symbol::slack();

    // Add the normal row to the tableau -- when artifical
    // variable is minimized to 0 (if possible)
    // this row remains in the tableau to maintain the constraint
    // we are trying to add.
    rows_[av] = r;

    // Try to optimize artificial row to 0.
    // Note we are *not* optimizing the real objective, but optimizing
    // the artificial objective to see if the error in the constraint
    // we are adding can be set to 0.
    artificial_ = r;
    optimize(artificial_);
    bool success = near_zero(artificial_.constant());
    artificial_ = row{};

    // Find another variable in this row and pivot, so that av becomes
    // parametric.
    // If there isn't another variable in the row then the tableau contains
    // the equation av = 0  -- just delete av's row
    auto it = rows_.find(av);
    if (it != rows_.end()) {
        row tmp{std::move(it->second)};
        rows_.erase(it);
        if (tmp.is_constant())
            return success;

        auto entering = pivotable_symbol(tmp);
        assert(!entering.is_nil());
        if (entering.is_nil())
            return false;

        tmp.solve_for(av, entering);
        substitute_out(entering, tmp);
        rows_[entering] = std::move(tmp);
    }

    for (auto& i : rows_)
        i.second.erase(av);

    objective_.erase(av);

    return success;
}

symbol simplex_solver::choose_subject(const expression_result& expr)
{
    for (auto& cell : expr.r.terms()) {
        if (cell.first.is_external())
            return cell.first;
    }

    if (expr.var1.is_pivotable()) {
        if (expr.r.coefficient(expr.var1) < 0.0)
            return expr.var1;
    }
    if (expr.var2.is_pivotable()) {
        if (expr.r.coefficient(expr.var2) < 0.0)
            return expr.var2;
    }
    return {};
}

simplex_solver& simplex_solver::add_edit_var(const variable& v, strength s)
{
    if (has_edit_var(v))
        throw duplicate_edit_variable();

    if (s.is_required())
        throw bad_required_strength();

    constraint cn{v, relation::eq, s};
    add_constraint(cn);
    auto ev = constraints_[cn];
    edits_[v] = edit_info{cn, ev.marker, ev.other, 0.0};

    return *this;
}

void simplex_solver::add_edit_vars(std::initializer_list<variable> list,
                                   strength s)
{
    for (auto&& v : list)
        add_edit_var(v, s);
}

void simplex_solver::remove_edit_var(const variable& v)
{
    auto found = edits_.find(v);
    if (found == edits_.end())
        throw unknown_edit_variable();

    remove_constraint(found->second.c);
    edits_.erase(found);
}

void simplex_solver::remove_edit_vars(std::initializer_list<variable> list)
{
    for (auto&& v : list)
        remove_edit_var(v);
}

bool simplex_solver::has_edit_var(const variable& v) const
{
    return edits_.count(v) != 0;
}

bool simplex_solver::has_variable(const variable& v) const
{
    return vars_.count(v) != 0;
}

void simplex_solver::update_external_variables()
{
    for (auto& v : vars_) {
        auto& var = const_cast<variable&>(v.first);
        auto row_it = rows_.find(v.second);
        if (row_it != rows_.end())
            var.set_value(row_it->second.constant());
    }
}

void simplex_solver::auto_update(bool flag)
{
    auto_update_ = flag;
    autoupdate();
}

symbol simplex_solver::get_var_symbol(const variable& v)
{
    auto it = vars_.find(v);
    if (it != vars_.end())
        return it->second;

    return vars_[v] = symbol::external();
}

void simplex_solver::substitute_out(const symbol& s, const row& r)
{
    for (auto& i : rows_) {
        i.second.substitute_out(s, r);
        if (i.first.is_restricted() && i.second.constant() < 0.0)
            infeasible_rows_.emplace_back(i.first);
    }
    objective_.substitute_out(s, r);
    artificial_.substitute_out(s, r);
}

void simplex_solver::optimize(const row& objective)
{
    while (true) {
        // Find the first negative coefficient in the objective function
        // (ignoring the non-pivotable dummy variables).  If all
        // coefficients are positive we're done.
        auto entry = symbol{};
        for (auto& p : objective.terms()) {
            if (!p.first.is_dummy() && p.second < 0.0) {
                entry = p.first;
                break;
            }
        }

        // If all coefficients were positive (or if the objective
        // function has no pivotable variables) we are at an optimum.
        if (entry.is_nil())
            return;

        // Choose which variable to move out of the basis.
        // Only consider pivotable basic variables
        // (i.e. restricted, non-dummy variables).
        auto exit = rows_.end();
        auto min_ratio = std::numeric_limits<double>::max();
        auto r = 0.0;
        for (auto i = rows_.begin(); i != rows_.end(); ++i) {
            auto var = i->first;
            if (var.is_pivotable()) {
                auto& expr = i->second;
                auto coeff = expr.coefficient(entry);

                if (coeff >= 0) // Only consider negative coefficients
                    continue;

                r = -expr.constant() / coeff;
                if (r < min_ratio
                    || (approx(r, min_ratio) && var < exit->first)) {
                    min_ratio = r;
                    exit = i;
                }
            }
        }

        if (exit == rows_.end())
            throw internal_error("objective function is unbounded.");

        // Pivot the entering symbol into the basis.
        auto exit_sym = exit->first;
        row tmp{std::move(exit->second)};
        rows_.erase(exit);
        tmp.solve_for(exit_sym, entry);
        substitute_out(entry, tmp);
        rows_[entry] = std::move(tmp);
    }
}

void simplex_solver::dual_optimize()
{
    while (!infeasible_rows_.empty()) {
        auto leaving = infeasible_rows_.back();
        infeasible_rows_.pop_back();
        auto it = rows_.find(leaving);
        if (it == rows_.end() || it->second.constant() >= 0.0)
            continue;

        auto& r = it->second;
        symbol entering;
        double min_ratio = std::numeric_limits<double>::max();
        for (auto& cell : r.terms()) {
            if (cell.second > 0.0 && !cell.first.is_dummy()) {
                double coeff = objective_.coefficient(cell.first);
                double r = coeff / cell.second;
                if (r < min_ratio) {
                    min_ratio = r;
                    entering = cell.first;
                }
            }
        }

        if (entering.is_nil())
            throw internal_error("Dual optimize failed.");

        // Pivot the entering symbol into the basis
        row tmp{std::move(it->second)};
        rows_.erase(it);
        tmp.solve_for(leaving, entering);
        substitute_out(entering, tmp);
        rows_[entering] = std::move(tmp);
    }
}

std::unordered_map<symbol, row>::iterator
simplex_solver::get_marker_leaving_row(const symbol& marker)
{
    auto dmax = std::numeric_limits<double>::max();
    auto r1 = dmax;
    auto r2 = dmax;
    auto end = rows_.end();
    auto first = end;
    auto second = end;
    auto third = end;
    for (auto it = rows_.begin(); it != end; ++it) {
        double c = it->second.coefficient(marker);
        if (c == 0.0)
            continue;
        if (it->first.is_external()) {
            third = it;
        } else if (c < 0.0) {
            double r = -it->second.constant() / c;
            if (r < r1) {
                r1 = r;
                first = it;
            }
        } else {
            double r = it->second.constant() / c;
            if (r < r2) {
                r2 = r;
                second = it;
            }
        }
    }
    if (first != end)
        return first;

    if (second != end)
        return second;

    return third;
}

void simplex_solver::autoupdate()
{
    if (auto_update_)
        update_external_variables();
}

void simplex_solver::add(row& r, symbol sym, double coeff)
{
    auto found = rows_.find(sym);
    if (found != rows_.end())
        r += found->second * coeff;
    else
        r += sym * coeff;
}

void simplex_solver::change_strength(const constraint& c, strength s)
{
    auto ic = constraints_.find(c);
    if (ic == constraints_.end())
        return;

    auto& evs = ic->second;
    if (!evs.marker.is_error())
        throw bad_required_strength{};

    auto& cn = const_cast<constraint&>(ic->first);
    double old_coeff = cn.get_strength();
    cn.set_strength(s);
    double diff = cn.get_strength() - old_coeff;

    if (near_zero(diff))
        return;

    add(objective_, evs.marker, diff);
    add(objective_, evs.other, diff);
    optimize(objective_);
    autoupdate();
}

} // namespace rhea
