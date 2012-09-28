//---------------------------------------------------------------------------
/// \file   tableau.hpp
/// \brief  Tableau for holding and manipulating linear expressions
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

#include <unordered_map>
#include <iostream>
#include "errors.hpp"
#include "variable.hpp"
#include "linear_expression.hpp"

namespace rhea {

class tableau
{
public:
    typedef std::unordered_map<variable, variable_set>      columns_map;
    typedef std::unordered_map<variable, linear_expression> rows_map;

public:
    // Variable v has been removed from an Expression.  If the
    // Expression is in a tableau the corresponding basic variable is
    // subject (or if subject is nil then it's in the objective function).
    // Update the column cross-indices.
    void note_removed_variable(const variable& v, const variable& subj);

    void note_added_variable(const variable& v, const variable& subj);

    bool is_valid() const;

void print() const
{
    std::cout << "Columns:" << std::endl;
    for (auto& p : columns_)
    {
        std::cout << p.first.description() << " => ";
        for (auto& v : p.second)
            std::cout << v.description() << " , ";

        std::cout << std::endl;
    }

    std::cout << "Rows:" << std::endl;
    for (auto& p : rows_)
    {
        std::cout << p.first.description() << " => " << p.second.to_string() << std::endl;
    }
}

public:
    tableau() { }

    virtual ~tableau() { }

    void add_row(variable v, const linear_expression& e);

    variable remove_column(variable v);

    linear_expression remove_row(variable v);

    void substitute_out(variable old_var, const linear_expression& e);

    const columns_map& columns() const { return columns_; }

    const rows_map& rows() const { return rows_; }

    bool columns_has_key(variable subj) const
        { return columns_.count(subj) > 0; }

    const linear_expression& row_expression(const variable& v) const
    {
        auto i (rows_.find(v));
        if (i == rows_.end())
            throw row_not_found();

        return i->second;
    }

    linear_expression& row_expression(const variable& v)
    {
        auto i (rows_.find(v));
        if (i == rows_.end())
            throw row_not_found();

        return i->second;
    }

    bool is_basic_var(const variable& v) const
        { return rows_.count(v) > 0; }

    bool is_parametric_var(const variable& v) const
        { return rows_.count(v) == 0; }

protected:
    // _columns is a mapping from variables which occur in expressions to the
    // set of basic variables whose expressions contain them
    // i.e., it's a mapping from variables in expressions (a column) to the
    // set of rows that contain them
    columns_map     columns_;

    // _rows maps basic variables to the expressions for that row in the tableau
    rows_map        rows_;

    // the collection of basic variables that have infeasible rows
    // (used when reoptimizing)
    variable_set    infeasible_rows_;

    // the set of rows where the basic variable is external
    // this was added to the C++ version to reduce time in SetExternalVariables()
    variable_set    external_rows_;

    // the set of external variables which are parametric
    // this was added to the C++ version to reduce time in SetExternalVariables()
    variable_set    external_parametric_vars_;

};

} // namespace rhea

