//---------------------------------------------------------------------------
/// \file   variable.hpp
/// \brief  A variable as used in an expression
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <memory>

#include "float_variable.hpp"
#include "link_variable.hpp"
#include "action_variable.hpp"

namespace rhea
{

/** This tag is used in \a variable to link to external variables. */
struct linked
{
};

/** A variable as used in an expression.
 * Variables don't use the normal C++ copy semantics: objects are actually
 * counted references to an \a abstract_variable.  The following example
 * illustrates this:
 * \code

variable x(1), y(0);

y = x;
// y is now 1

x.set_value(2);
// both x and y are now 2

 * \endcode
 *
 * Also note that a variable is nullable.  A variable that has been
 * constructed without a type cannot be used in expressions.
 *
 * Another caveat: "x == y" is not a boolean, but a \a constraint.  There are
 * two ways to compare two variables, depending on whether you want to test for
 * equality or equivalence:
 * \code

variable x(2), y(x), z(2);

x.is(y); // True: y was constructed from x
x.is(z); // False: x and z both have the value 2, but they are different
variables

x.value() == y.value(); // True
x.value() == z.value(); // Also true

 * \endcode
 * */
class variable
{
public:
    variable()
        : p_{std::make_shared<float_variable>(0.0)}
    {
    }

    /** An explicit nil variable.
     *  This function only serves to make code more readable. */
    static variable nil_var() { return {nil_()}; }

    /** Wrap an abstract variable on the heap.
     * \param p  Shared pointer to a variable.
     */
    template <typename T>
    variable(std::shared_ptr<T>&& p)
        : p_{std::move(p)}
    {
    }

    /** "Copy" a variable.
     *  The resulting variable won't be a true copy, but rather another
     *  counted reference to the same variable. */
    variable(const variable& copy)
        : p_{copy.p_}
    {
    }

    /** Move constructor. */
    variable(variable&& copy)
        : p_{std::move(copy.p_)}
    {
    }

    /** Create a new floating pointe variable.
     * \param value  The variable's initial value
     */
    variable(int value)
        : p_{std::make_shared<float_variable>(value)}
    {
    }

    /** Create a new floating point variable.
     * \param value  The variable's initial value
     */
    variable(unsigned int value)
        : p_{std::make_shared<float_variable>(value)}
    {
    }

    /** Create a new floating point variable.
     * \param value  The variable's initial value
     */
    variable(float value)
        : p_{std::make_shared<float_variable>(value)}
    {
    }

    /** Create a new floating point variable.
     * \param value  The variable's initial value
     */
    variable(double value)
        : p_{std::make_shared<float_variable>(value)}
    {
    }

    /** Create variable that is linked to an existing integer.
     *  It is up to you to make sure the linked variable isn't destroyed
     *  while the solver is still using it.
     * \code
     * int some_integer;
     * rhea::variable x {some_integer, rhea::linked()};
     * x = 8;
     * // some_integer is now 8
     * \endcode
     *
     * \param value  This variable will be automatically updated
     */
    variable(int& value, const linked&)
        : p_{std::make_shared<link_int>(value)}
    {
    }

    /** Create variable that is linked to an existing float.
     *  It is up to you to make sure the linked variable isn't destroyed
     *  while the solver is still using it.
     * \code
     * float some_float;
     * rhea::variable x {some_float, rhea::linked()};
     * x = 8.0f;
     * // some_float is now 8
     * \endcode
     *
     * \param value  This variable will be automatically updated
     */
    variable(float& value, const linked&)
        : p_{std::make_shared<link_variable<float>>(value)}
    {
    }

    /** Create variable that is linked to an existing double.
     *  It is up to you to make sure the linked variable isn't destroyed
     *  while the solver is still using it.
     * \param value  This variable will be automatically updated
     */
    variable(double& value, const linked&)
        : p_{std::make_shared<link_variable<double>>(value)}
    {
    }

    /** Create a variable that calls a function whenever it is updated.
     * \code
     * rhea::variable x {[](double v)
     *                   { std::cout << "x is " << v << std::endl; }};
     * x = 6.5;
     * // prints: "x is 6.5"
     * \endcode
     */
    variable(std::function<void(double)> callback, double init_val = 0.0)
        : p_{std::make_shared<action_variable>(callback, init_val)}
    {
    }

    variable& operator=(const variable& assign)
    {
        p_ = assign.p_;
        return *this;
    }

    variable& operator=(variable&& move)
    {
        p_ = std::move(move.p_);
        return *this;
    }

    /** Get the value of this variable. */
    double value() const { return p_ ? p_->value() : 0.0; }

    /** Get the value of this variable, converted to an integer. */
    int int_value() const { return p_->int_value(); }

    /** Set this variable to a new value. */
    void set_value(double x) { p_->set_value(x); }

    /** Check if this is a nil variable. */
    bool is_nil() const { return p_ == nullptr; }

    /** Get a string representation of the variable type.
     * Normal variables will return "var", linked variables will return
     * "link". */
    std::string to_string() const
    {
        return is_nil() ? "NIL" : p_->to_string();
    }

    /** Check if two variables refer to the same abstract_variable.
     *  This will not return 'true' for two distinct variables that happen
     *  to have the same value.  Example:
     * \code
     variable x(3), y(3), z;
     x.is(y); // False!
     z = x;  // z now refers to x
     z.set_value(5);
     x.is(z); // True (x.value() == 5 as well)
     * \endcode
     */
    bool is(const variable& x) const { return p_ == x.p_; }

    size_t hash() const
    {
        assert(!is_nil());
        return std::hash<std::shared_ptr<abstract_variable>>()(p_);
    }

private:
    struct nil_
    {
    };

    variable(const nil_&) {}

private:
    /** Reference counted pointer to the actual variable. */
    std::shared_ptr<abstract_variable> p_;
};

} // namespace rhea

//-------------------------------------------------------------------------

namespace std
{

template <>
struct hash<rhea::variable> : public unary_function<rhea::variable, size_t>
{
    size_t operator()(const rhea::variable& v) const { return v.hash(); }
};

template <>
struct equal_to<rhea::variable>
    : public binary_function<rhea::variable, rhea::variable, bool>
{
    bool operator()(const rhea::variable& a, const rhea::variable& b) const
    {
        return a.is(b);
    }
};

/** Get a string representation of a variable. */
inline string to_string(const rhea::variable& v)
{
    return v.is_nil() ? "NIL" : to_string(v.value());
}

} // namespace std
