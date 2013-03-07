//---------------------------------------------------------------------------
/// \file   constraint.hpp
/// \brief  Base class for constraints
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

#include <list>
#include <memory>
#include "abstract_constraint.hpp"
#include "linear_equation.hpp"
#include "linear_inequality.hpp"

namespace rhea {

/** An equation or inequality involving one or more variables.
 * Constraints can be defined as "normal" C++ expressions:
 * \code

 variable x (1), y (2);

 constraint a (x + 4 <= y * 2);
 constraint b (x * 2 == y * 3);

 * \endcode
 */
class constraint
{
public:
    constraint() { }

    template <typename t>
    constraint(std::shared_ptr<t>&& p)
        : p_(std::move(p))
    { }

    constraint(const linear_equation& eq)
        : p_(std::make_shared<linear_equation>(eq))
    { }

    constraint(const linear_equation& eq, strength s, double weight = 1)
        : p_(std::make_shared<linear_equation>(eq.expression(), std::move(s), weight))
    { }

    constraint(const linear_inequality& eq)
        : p_(std::make_shared<linear_inequality>(eq))
    { }

    constraint(const linear_inequality& eq, strength s, double weight = 1)
        : p_(std::make_shared<linear_inequality>(eq.expression(), std::move(s), weight))
    { }

public:
    linear_expression expression() const
        { return p_->expression(); }

    bool is_edit_constraint() const
        { return p_->is_edit_constraint(); }

    bool is_inequality() const
        { return p_->is_inequality(); }

    bool is_strict_inequality() const
        { return p_->is_strict_inequality(); }

    bool is_required() const
        { return p_->is_required(); }

    bool is_stay_constraint() const
        { return p_->is_stay_constraint(); }

    const strength& get_strength() const
        { return p_->get_strength(); }

    double weight() const
        { return p_->weight(); }

    const variable_set& read_only_variables() const
        { return p_->read_only_variables(); }

    bool is_satisfied() const
        { return p_->is_satisfied(); }

    bool is_in_solver() const
        { return p_->is_in_solver(); }

    bool is_okay_for_simplex_solver() const
        { return p_->is_okay_for_simplex_solver(); }

    bool is_read_only(const variable& v) const
        { return p_->is_read_only(v); }

    void change_strength(const strength& new_strength)
        { p_->change_strength(new_strength); }

    void change_weight(double new_weight)
        { p_->change_weight(new_weight); }

    constraint& mark_as_readonly(const variable& v)
        { p_->mark_as_readonly(v); return *this; }

    constraint& mark_as_readonly(const variable_set& vars)
        { p_->mark_as_readonly(vars); return *this; }

    symbolic_weight get_symbolic_weight() const
        { return p_->get_symbolic_weight(); }

    double adjusted_symbolic_weight() const
        { return p_->adjusted_symbolic_weight(); }

    void set_strength(const strength& n)
        { p_->set_strength(n); }

    void set_weight(double n)
        { p_->set_weight(n); }

    void add_to(solver& s)
        { p_->add_to(s); }

    void remove_from(solver& s)
        { p_->remove_from(s); }

    template <typename t>
    t& as() { return dynamic_cast<t&>(*p_); }

    template <typename t>
    const t& as() const { return dynamic_cast<const t&>(*p_); }

    bool is_nil() const { return !p_; }

public:
    constraint& operator= (const constraint& assign)
        {
            p_ = assign.p_;
            return *this;
        }

    bool operator== (const constraint& other) const
        { return p_ == other.p_; }

    bool operator!= (const constraint& other) const
        { return p_ != other.p_; }

    size_t hash() const
        { return std::hash<std::shared_ptr<abstract_constraint>>()(p_); }

private:
    std::shared_ptr<abstract_constraint> p_;
};


/** Convenience typedef for bundling constraints. */
typedef std::list<constraint> constraint_list;

} // namespace rhea


namespace std {

/** Hash function, required for std::unordered_map. */
template<>
struct hash<rhea::constraint>
    : public unary_function<rhea::constraint, size_t>
{
    size_t operator() (const rhea::constraint& c) const
        { return c.hash(); }
};

} // namespace std
