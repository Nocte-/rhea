//---------------------------------------------------------------------------
/// \file   objective_variable.hpp
/// \brief  The objective for a solver to work towards
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
#include "tableau.hpp"

namespace rhea {

/** A special variable that is used internally by the solver as the
 ** objective to solve for. */
class objective_variable : public abstract_variable
{
public:
    objective_variable() { }

    virtual ~objective_variable() { }

    virtual bool is_external() const    { return false; }
    virtual bool is_pivotable() const   { return false; }
    virtual bool is_restricted() const  { return false; }

    std::string to_string() const { return "objective"; }
};

} // namespace rhea
