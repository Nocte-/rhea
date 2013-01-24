//---------------------------------------------------------------------------
/// \file   iostream.hpp
/// \brief  Standard Library iostream support for variables
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

#include <iostream>
#include <string>

#include "float_variable.hpp"

namespace std {

inline
ostream& operator<< (ostream& str, const rhea::variable& v)
{
    return str << v.to_string();
}

inline
istream& operator>> (istream& str, rhea::variable& v)
{
    double temp;
    str >> temp;
    v.set_value(temp);
    return str;
}

} // namespace std

