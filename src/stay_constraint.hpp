//---------------------------------------------------------------------------
/// \file   stay_constraint.hpp
/// \brief  Stay constraint
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

class stay_constraint : public edit_or_stay_constraint
{
public:
    stay_constraint(const variable& v, strength s = strength::weak(),
                    double weight = 1.0)
        : edit_or_stay_constraint(v, s, weight)
    { }

    virtual ~stay_constraint() { }

    virtual bool is_stay_constraint() const
        { return true; }

    virtual bool is_satisfied() const
        { return false; }
};

} // namespace rhea
