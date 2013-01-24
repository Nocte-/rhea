//---------------------------------------------------------------------------
/// \file   edit_constraint.hpp
/// \brief  Edit constraint
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

#include "edit_or_stay_constraint.hpp"

namespace rhea {

/** Edit constraints are added to a tableau on a variable, so that a
 ** new value can be suggested for that variable later on. */
class edit_constraint : public edit_or_stay_constraint
{
public:
    edit_constraint(const variable& v, const strength& s = strength::strong(),
                    double weight = 1.0)
        : edit_or_stay_constraint(v, s, weight)
    { }

    virtual ~edit_constraint() { }

    virtual bool is_edit_constraint() const
        { return true; }

    virtual bool is_satisfied() const
        { return false; }
};

} // namespace rhea
