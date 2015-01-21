//---------------------------------------------------------------------------
/// \file   iostream.hpp
/// \brief  Standard Library iostream support for variables
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "constraint.hpp"
#include "linear_expression.hpp"
#include "simplex_solver.hpp"
#include "strength.hpp"
#include "variable.hpp"

namespace std
{

inline ostream& operator<<(ostream& str, const rhea::variable& v)
{
    if (v.is_nil())
        return str << "NIL";

    return str << "{" << v.to_string() << ":" << v.value() << "}";
}

inline ostream& operator<<(ostream& str, const rhea::symbol& s)
{
    if (s.is_nil())
        return str << "--";

    return str << static_cast<char>(s.type()) << s.id();
}

template <typename T>
ostream& operator<<(ostream& str, const rhea::expression<T>& e)
{
    for (auto& t : e.terms())
        str << t.first << "*" << t.second << " + ";

    return str << e.constant();
}

inline ostream& operator<<(ostream& str, const rhea::strength& s)
{
    if (s.is_required())
        return str << "required";

    if (s >= rhea::strength::strong())
        return str << "strong";

    if (s >= rhea::strength::medium())
        return str << "medium";

    return str << "weak";
}

inline ostream& operator<<(ostream& str, const rhea::constraint& c)
{
    str << c.expr();
    switch (c.oper().type()) {
    case rhea::relation::eq:
        str << " == 0";
        break;
    case rhea::relation::leq:
        str << " <= 0";
        break;
    case rhea::relation::geq:
        str << " >= 0";
        break;
    default:
        assert(false);
    }
    return str << " | " << c.get_strength();
}

inline ostream& operator<<(ostream& str, const rhea::simplex_solver& s)
{
    str << "Variables:\n";
    for (auto&& v : s.vars_)
        str << v.first << " : " << v.second << "\n";

    str << "Constraints:\n";
    for (auto&& c : s.constraints_)
        str << c.second.marker << " : " << c.first << "\n";

    str << "Rows:\n";
    for (auto&& r : s.rows_)
        str << r.first << " : " << r.second << "\n";

    str << "Infeasible:\n";
    for (auto&& r : s.infeasible_rows_)
        str << r << "  ";

    return str << "\nObjective:\n" << s.objective_ << "\n";
}

} // namespace std
