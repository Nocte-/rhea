//---------------------------------------------------------------------------
/// \file   symbol.hpp
/// \brief  A symbol in the tableau
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <functional>

namespace rhea
{

/** Symbols are variables in the tableau. */
class symbol
{
public:
    enum class type_t : char {
        /** An uninitialized variable. */
        nil,

        /** An external variable.
         * The value of these variables are copied from the tableau back out
         * to the \a rhea::variable objects by calling
         * \a simplex_solver::update_external_variables(). */
        external = 'v',

        /** A slack variable.
         * Slack variables are used in inequality constraints. The tableau
         * only holds equations, so all inequalities \f$l \geq 0\f$ (where
         * \f$l\f$ is any linear expression) are first rewritten as
         * \f$l = s \wedge s \geq 0\f$, where \f$s\f$ is the slack variable.
         */
        slack = 's',

        /** An error variable.
         * A required constraint can be turned into a non-required one by
         * adding two non-negative error variables:
         * \f$l + \delta^+ - \delta^- = 0\f$*/
        error = 'e',

        /** A dummy variable.
         * An inequality has a slack variable that can be used as a marker. Non-
         * required constraints have error variables. Required equalities have
         * neither, so we introduce a dummy variable that always remains zero,
         * and only serves as a marker to find back the constraint's influences
         * in the tableau. */
        dummy = 'd'
    };

    symbol()
        : type_{type_t::nil}
    {
    }

    symbol(type_t t)
        : id_{count_++}
        , type_{t}
    {
    }

    symbol(const symbol&) = default;
    symbol(symbol&&) = default;
    symbol& operator=(const symbol&) = default;
    symbol& operator=(symbol&&) = default;

    static symbol external() { return {type_t::external}; }
    static symbol slack() { return {type_t::slack}; }
    static symbol error() { return {type_t::error}; }
    static symbol dummy() { return {type_t::dummy}; }

    uint32_t id() const { return id_; }
    type_t type() const { return type_; }

    bool is_nil() const { return type_ == type_t::nil; }
    bool is_external() const { return type_ == type_t::external; }
    bool is_slack() const { return type_ == type_t::slack; }
    bool is_error() const { return type_ == type_t::error; }
    bool is_dummy() const { return type_ == type_t::dummy; }

    bool is_restricted() const { return !is_external(); }
    bool is_unrestricted() const { return !is_restricted(); }
    bool is_pivotable() const { return is_slack() || is_error(); }

    bool is(const symbol& c) const { return id_ == c.id_; }

    bool operator==(const symbol& c) const { return id_ == c.id_; }
    bool operator!=(const symbol& c) const { return id_ != c.id_; }
    bool operator<(const symbol& c) const { return id_ < c.id_; }

private:
    static uint32_t count_;
    uint32_t id_;
    type_t type_;
};

} // namespace rhea

namespace std
{

template <>
struct hash<rhea::symbol> : public unary_function<rhea::symbol, size_t>
{
    size_t operator()(const rhea::symbol& v) const { return v.id(); }
};

template <>
struct equal_to<rhea::symbol>
    : public binary_function<rhea::symbol, rhea::symbol, bool>
{
    bool operator()(const rhea::symbol& a, const rhea::symbol& b) const
    {
        return a.is(b);
    }
};

} // namespace std
