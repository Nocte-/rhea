//---------------------------------------------------------------------------
/// \file   iostream.hpp
/// \brief  Standard Library iostream support for variables
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
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
