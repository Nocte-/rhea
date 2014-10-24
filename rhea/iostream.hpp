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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "float_variable.hpp"
#include "linear_expression.hpp"
#include "tableau.hpp"

namespace std
{

inline ostream& operator<<(ostream& str, const rhea::variable& v)
{
    if (v.is_float())
        return str << v.to_string();
        //return str << "[v" << v.id() << ":" << v.to_string() << "]";

    return str << "[" << v.to_string() << v.id() << "]";
}

inline istream& operator>>(istream& str, rhea::variable& v)
{
    double temp;
    str >> temp;
    v.set_value(temp);
    return str;
}

inline ostream& operator<<(ostream& str, const rhea::linear_expression& v)
{
    for (auto& t : v.terms())
        str << t.first << "*" << t.second << " + ";

    return str << v.constant();
}

inline ostream& operator<<(ostream& str, const rhea::tableau& v)
{
    str << "Tableau columns" << std::endl;
    for (auto& col : v.columns())
    {
        str << "  " << col.first << " : ";
        for (auto& var : col.second)
            str << var << "  ";

        str << std::endl;
    }

    str << "Tableau rows" << std::endl;
    for (auto& row : v.rows())
        str << "  " << row.first << " : " << row.second << std::endl;

    return str;
}

} // namespace std
