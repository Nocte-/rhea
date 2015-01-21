//---------------------------------------------------------------------------
/// \file   constraint.hpp
/// \brief  A linear equation or inequality
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <memory>
#include "linear_expression.hpp"
#include "relation.hpp"
#include "strength.hpp"

namespace rhea
{

class simplex_solver;

/** An equation or inequality involving one or more variables.
 * Constraints can be defined as "normal" C++ expressions:
 * \code

 variable x {1}, y {2};
 constraint a {x + 4 >= y * 2};
 constraint b {x * 2 == y * 3};

 assert(a.is_satisfied());

 * \endcode
 *
 * Constraints also have a strength, a value that allows the Cassowary
 * algorithm to find a solution between conflicting constraints. The pipe
 * operator can be used to set a constraint's strength:
 * \code

 auto a = x + 4 >= y * 2 | strength::medium();
 auto b = x * 2 == y * 3 | strength::weak(4); // 4 times the default weight

 * \endcode
 */
class constraint
{
    friend class simplex_solver;

public:
    constraint() {}

    constraint(const linear_expression& e, relation op,
               strength str = strength::required())
        : p_(std::make_shared<data>(e, op, str))
    {
    }

    constraint(linear_expression&& e, relation op,
               strength str = strength::required())
        : p_(std::make_shared<data>(std::move(e), op, str))
    {
    }

    /** This constructor is used to change the strength of an existing
     ** constraint.
     * \sa operator| */
    constraint(const constraint& c, strength str)
        : p_(std::make_shared<data>(c.expr(), c.oper(), str))
    {
    }

    /** Build a constraint from a variable, a relation, and an expression. */
    constraint(const variable& lhs, relation op, linear_expression rhs,
               strength str = strength::required())
        : p_(std::make_shared<data>(std::move(rhs -= lhs), op, str))
    {
    }

    /** Build a constraint from a relation between two expressions. */
    constraint(const linear_expression& lhs, relation op,
               linear_expression rhs, strength str = strength::required())
        : p_(std::make_shared<data>(std::move(rhs -= lhs), op, str))
    {
    }

    constraint(const constraint&) = default;
    constraint(constraint&&) = default;
    constraint& operator= (const constraint&) = default;
    constraint& operator= (constraint&&) = default;


    const linear_expression& expr() const { return p_->expr_; }
    relation oper() const { return p_->op_; }
    strength get_strength() const { return p_->str_; }
    void set_strength(strength str) const { p_->str_ = str; }

    bool is(const constraint& c) const { return p_ == c.p_; }
    bool is_nil() const { return p_ == nullptr; }
    bool is_required() const { return get_strength().is_required(); }
    bool is_inequality() const { return oper().type() != relation::eq; }

    bool is_satisfied() const
    {
        switch (oper().type()) {
        case relation::eq:
            return p_->expr_.evaluate() == 0;
        case relation::leq:
            return p_->expr_.evaluate() <= 0;
        case relation::geq:
            return p_->expr_.evaluate() >= 0;
        }
        return false;
    }

    bool operator==(const constraint& c) const { return p_ == c.p_; }
    bool operator!=(const constraint& c) const { return p_ != c.p_; }
    bool operator<(const constraint& c) const { return p_ < c.p_; }

    size_t hash() const { return std::hash<std::shared_ptr<data>>()(p_); }

private:
    struct data
    {
        data(linear_expression&& e, relation o, strength s)
            : expr_{std::move(e)}
            , op_{o}
            , str_{s}
        {
        }

        data(const linear_expression& e, relation o, strength s)
            : expr_{e}
            , op_{o}
            , str_{s}
        {
        }

        linear_expression expr_;
        relation op_;
        strength str_;
    };

    std::shared_ptr<data> p_;
};

//---------------------------------------------------------------------------

inline constraint operator==(const linear_expression& first,
                             const linear_expression& second)
{
    return {first - second, relation::eq};
}

inline constraint operator==(const linear_expression& le, const variable& v)
{
    return le == linear_expression{v};
}

inline constraint operator==(const linear_expression& le, double constant)
{
    return le == linear_expression{constant};
}

inline constraint operator==(const variable& v, double constant)
{
    return linear_expression{v} == linear_expression{constant};
}

inline constraint operator==(double constant, const variable& v)
{
    return linear_expression{v} == linear_expression{constant};
}

//---------------------------------------------------------------------------

inline constraint operator>=(const linear_expression& first,
                             const linear_expression& second)
{
    return {first - second, relation::geq};
}

inline constraint operator>=(const linear_expression& le, const variable& v)
{
    return le >= linear_expression{v};
}

inline constraint operator>=(const linear_expression& le, double constant)
{
    return le >= linear_expression(constant);
}

inline constraint operator>=(const variable& v, double constant)
{
    return linear_expression{v} >= linear_expression(constant);
}

//---------------------------------------------------------------------------

inline constraint operator<=(const linear_expression& first,
                             const linear_expression& second)
{
    return {first - second, relation::leq};
}

inline constraint operator<=(const linear_expression& le, const variable& v)
{
    return le <= linear_expression{v};
}

inline constraint operator<=(const linear_expression& le, double constant)
{
    return le <= linear_expression(constant);
}

inline constraint operator<=(const variable& v, double constant)
{
    return linear_expression{v} <= linear_expression(constant);
}

//---------------------------------------------------------------------------

inline constraint operator|(const constraint& c, strength s)
{
    return {c, s};
}

} // namespace rhea

//---------------------------------------------------------------------------

namespace std
{

/** Hash function, required for std::unordered_map. */
template <>
struct hash<rhea::constraint> : public unary_function<rhea::constraint, size_t>
{
    size_t operator()(const rhea::constraint& v) const { return v.hash(); }
};

} // namespace std
