//---------------------------------------------------------------------------
/// \file   action_variable.hpp
/// \brief  A variable that calls a function whenever it changes
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <functional>
#include "float_variable.hpp"

namespace rhea
{

/** A variable that calls a function whenever it changes.
 */
class action_variable : public float_variable
{
public:
    action_variable(std::function<void(double)> callback, double value)
        : float_variable{value}
        , callback_{callback}
    {
    }

    virtual ~action_variable() {}

    virtual void set_value(double new_value)
    {
        value_ = new_value;
        callback_(new_value);
    }

    virtual void change_value(double new_value) { set_value(new_value); }

protected:
    std::function<void(double)> callback_;
};

} // namespace rhea
