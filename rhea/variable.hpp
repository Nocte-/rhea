//---------------------------------------------------------------------------
/// \file   variable.hpp
/// \brief  A variable as used in an expression
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
// Copyright 2012, 2013, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "approx.hpp"
#include "abstract_variable.hpp"
#include "float_variable.hpp"

namespace rhea {

/** A variable as used in an expression.
 * Variables don't use the normal C++ copy semantics: objects are actually
 * counted references to an abstract_variable.  The following example
 * illustrates this:
 *
 * \code

variable x(1), y(0);

y = x;
// y is now 1

x.set_value(2);
// both x and y are now 2

 * \endcode
 * Also note that a variable is nullable.  A variable that has been
 * constructed without a type cannot be used in expressions.
 *
 * Another caveat: "x == y" is not a boolean, but a linear_equality that
 * can be evaluated and used in constraints.  There are two ways to compare
 * two variables, depending on whether you want to test for equality or
 * equivalence:
 *
 * \code

variable x(2), y(x), z(2);

x.is(y); // True: y was constructed from x
x.is(z); // False: x and z both have the value 2, but they are different variables

x.value() == y.value(); // True
x.value() == z.value(); // Also true

 * \endcode
 * */
class variable
{
public:
    variable() { }

    /** An explicit nil variable.
     *  This function only serves to make code more readable. */
    static variable nil_var() { return variable(); }

    /** Wrap an abstract variable on the heap.
     *  This object will now take care of the variable's lifetime, so
     *  don't delete it later on. Example:
     * \code
     variable my_var (new objectve("az"));
     * \endcode
     * \param p  Pointer to a variable on the heap.
     */
    variable(abstract_variable* p)
        : p_(p)
    {
        assert(p != nullptr);
    }

    /** Wrap an abstract variable on the heap.
     * \param p  Shared pointer to a variable.
     */
    variable(std::shared_ptr<abstract_variable> p)
        : p_(std::move(p))
    { }

    /** "Copy" a variable.
     *  The resulting variable won't be a true copy, but rather another
     *  counted reference to the same variable. */
    variable(const variable& copy)
        : p_(copy.p_)
    { }

    /** Move constructor. */
    variable(variable&& copy)
        : p_(std::move(copy.p_))
    { }

    /** Create a new floating pointe variable.
     * \param value  The variable's initial value
     */
    variable(int value)
        : p_(std::make_shared<float_variable>(value))
    { }

    /** Create a new floating point variable.
     * \param value  The variable's initial value
     */
    variable(unsigned int value)
        : p_(std::make_shared<float_variable>(value))
    { }

    /** Create a new floating point variable.
     * \param value  The variable's initial value
     */
    variable(float value)
        : p_(std::make_shared<float_variable>(value))
    { }

    /** Create a new floating pointe variable.
     * \param value  The variable's initial value
     */
    variable(double value)
        : p_(std::make_shared<float_variable>(value))
    { }



    variable& operator= (const variable& assign)
        { p_ = assign.p_; return *this; }

    variable& operator= (variable&& move)
        { p_ = std::move(move.p_); return *this; }



    /** Check if this variable is of the type float_variable. */
    bool is_float() const
        { return p_->is_float(); }

    /** Check if this variable is used in the finite domain subsolver. */
    bool is_fd() const
        { return p_->is_fd(); }

    /** Check if this variable is a dummy variable. */
    bool is_dummy() const
        { return p_->is_dummy(); }

    /** Check if this variable is used outside the solver. */
    bool is_external() const
        { return p_->is_external(); }

    /** Check if this variable can be used as a pivot element in a tableau. */
    bool is_pivotable() const
        { return p_->is_pivotable(); }

    /** Check if this variable is restricted, or in other words, if it is
     ** a dummy or a slack variable. */
    bool is_restricted() const
        { return p_->is_restricted(); }

    /** Get the value of this variable. */
    double value() const
        { return p_->value(); }

    /** Get the value of this variable, converted to an integer. */
    int int_value() const
        { return p_->int_value(); }

    /** Set this variable to a new value. */
    void set_value(double x)
        { p_->set_value(x); }

    /** Change this variable's value. */
    void change_value(double x)
        { p_->change_value(x); }

    /** Check if this is a nil variable. */
    bool is_nil() const
        { return p_ == nullptr; }

    /** Calculate a hash value.
     *  This function is only used for placing variables in hash tables. */
    size_t hash() const
        { return std::hash<std::shared_ptr<abstract_variable>>()(p_); }

    /** Get a string representation of the value. */
    std::string to_string() const
        { return is_nil() ? "NIL" : p_->to_string(); }

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
    bool is(const variable& x) const
        { return p_ == x.p_; }

private:
    /** Reference counted pointer to the actual variable. */
    std::shared_ptr<abstract_variable> p_;
};


/** Convenience typedef for sets of variables. */
typedef std::unordered_set<variable>    variable_set;

} // namespace rhea

//-------------------------------------------------------------------------

namespace std {

/** Hash function, required for std::unordered_map and -set. */
template<>
struct hash<rhea::variable>
    : public unary_function<rhea::variable, size_t>
{
    size_t operator() (const rhea::variable& v) const
        { return v.hash(); }
};

/** Equality test, required for std::unordered_map and -set. */
template<>
struct equal_to<rhea::variable>
    : public binary_function<rhea::variable, rhea::variable, bool>
{
    bool operator()(const rhea::variable& a, const rhea::variable& b) const
        { return a.is(b); }
};

/**  */
inline
std::string to_string(const rhea::variable& v)
{
    return v.to_string();
}

} // namespace std
