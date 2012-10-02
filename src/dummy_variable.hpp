//---------------------------------------------------------------------------
/// \file   dummy_variable.hpp
/// \brief  Dummy placeholder used when solving a tableau
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

/** Dummy variables are inserted by the simplex solver as a marker when
 ** incrementally removing a required equality constraint.  */
class dummy_variable : public abstract_variable
{
public:
    dummy_variable() : abstract_variable() { }
    virtual ~dummy_variable() { }

    virtual bool is_dummy() const       { return true;  }
    virtual bool is_external() const    { return false; }
    virtual bool is_pivotable() const   { return false; }
    virtual bool is_restricted() const  { return true;  }

    virtual std::string to_string() const { return "dummy"; }
};

} // namespace rhea
