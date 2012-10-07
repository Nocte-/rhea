//---------------------------------------------------------------------------
/// \file   slack_variable.hpp
/// \brief  Slack variable
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

#include "abstract_variable.hpp"

namespace rhea {

/** Slack variables are used to turn inequalities into equations.
 * For example, this inequality:
 * \f$
 * 3x + 5y \leq 10
 * \f$
 * becomes the equation:
 * \f$
 * 3x + 5y + s = 10
 * \f$ by introducing the slack variable \f$s\f$.
 */
class slack_variable : public abstract_variable
{
public:
    slack_variable() : abstract_variable() { }
    ~slack_variable() { }

    virtual bool is_external() const    { return false; }
    virtual bool is_pivotable() const   { return true; }
    virtual bool is_restricted() const  { return true; }

    std::string to_string() const { return "slack"; }
};

} // namespace rhea
