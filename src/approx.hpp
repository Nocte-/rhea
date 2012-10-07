//---------------------------------------------------------------------------
/// \file   approx.hpp
/// \brief  See if two values are equal within a margin
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

#include <cmath>

namespace rhea {

/** Return true iff a and b are approximately the same. */
inline bool approx(double a, double b)
{
    const double epsilon = 1.0e-8;
    if (a > b) {
        return (a - b) < epsilon;
    } else {
        return (b - a) < epsilon;
    }
}

/** Return true iff a is almost zero. */
inline bool near_zero(double a)
{
    return approx(a, 0.0);
}

} // namespace rhea
