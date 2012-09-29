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
// Copyright 2012, nocte@hippie.nu
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

class variable
{
public:
    variable() { }

    static variable nil() { return variable(); }

    variable(abstract_variable* p)
        : p_(p)
    {
        assert(p != nullptr);
    }

    variable(std::shared_ptr<abstract_variable> p)
        : p_(std::move(p))
    { }

    variable(const variable& copy)
        : p_(copy.p_)
    { }

    variable(int value)
        : p_(std::make_shared<float_variable>(value))
    { }

    variable(unsigned int value)
        : p_(std::make_shared<float_variable>(value))
    { }

    variable(float value)
        : p_(std::make_shared<float_variable>(value))
    { }

    variable(double value)
        : p_(std::make_shared<float_variable>(value))
    { }

    variable(std::string name, double value = 0.0)
        : p_(std::make_shared<float_variable>(std::move(name), value))
    { }

  //  operator double() const
  //      { return value(); }

    bool is_float() const
        { return p_->is_float(); }

    bool is_fd() const
        { return p_->is_fd(); }

    bool is_dummy() const
        { return p_->is_dummy(); }

    bool is_external() const
        { return p_->is_external(); }

    bool is_pivotable() const
        { return p_->is_pivotable(); }

    bool is_restricted() const
        { return p_->is_restricted(); }

    double value() const
        { return p_->value(); }

    int int_value() const
        { return p_->int_value(); }

    void set_value(double x)
        { p_->set_value(x); }

    void change_value(double x)
        { p_->change_value(x); }

    std::string name() const
        { return is_nil() ? "NIL" : p_->name(); }

    bool is_nil() const
        { return p_ == nullptr; }

    size_t hash() const
        { return std::hash<std::shared_ptr<abstract_variable>>()(p_); }

    std::string to_string() const
        { return is_nil() ? "NIL" : p_->to_string(); }

    std::string description() const
        { return name() + ":" + to_string(); }

    bool operator== (const variable& other) const
        { return p_ == other.p_; }

    bool operator!= (const variable& other) const
        { return p_ != other.p_; }

private:
    bool operator< (const variable& other) const
        { return p_ < other.p_; }

    bool operator<= (const variable& other) const
        { return p_ < other.p_ || p_ == other.p_; }

    bool operator>= (const variable& other) const
        { return !(p_ < other.p_); }

    bool operator> (const variable& other) const
        { return (!(p_ < other.p_)) && p_ != other.p_; }

private:
    std::shared_ptr<abstract_variable> p_;
};

struct variable_less_by_value
    : public std::binary_function<bool, variable, variable>
{
    bool operator() (const variable& a, const variable& b) const
    {
        return a.value() < b.value();
    }
};

typedef std::unordered_map<variable, double>        variable_to_number_map;
typedef std::unordered_map<std::string, variable>   string_to_var_map;
typedef std::unordered_set<variable>                variable_set;

} // namespace rhea

namespace std {

/** Hash function, required for std::unordered_map. */
template<>
struct hash<rhea::variable>
    : public unary_function<rhea::variable, size_t>
{
    size_t operator() (const rhea::variable& v) const
        { return v.hash(); }
};

} // namespace std
