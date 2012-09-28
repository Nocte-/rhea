//---------------------------------------------------------------------------
/// \file   point.hpp
/// \brief  A convenience class for using 2D points in expressions
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

#include "variable.hpp"

namespace rhea {

class point
{
public:
    variable x;
    variable y;

public:
    point (variable ix, variable iy)
        : x (ix), y (iy)
    { }

    point& set_xy(double ix, double iy)
    {
        x = ix; y = iy;
        return *this;
    }

    double x_value() const { return x.value(); }
    double y_value() const { return y.value(); }
};

} // namespace rhea
