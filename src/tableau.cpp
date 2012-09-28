//---------------------------------------------------------------------------
// tableau.cpp
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
#include "tableau.hpp"

namespace rhea {

void tableau::add_row(variable var, const linear_expression& expr)
{
    assert(!var.is_nil());
    rows_[var] = expr;
    for (auto& p : expr.terms())
    {
        const variable& v (p.first);
        columns_[v].insert(var);
        if (v.is_external() && !is_basic_var(v))
            external_parametric_vars_.insert(v);
    }

    if (var.is_external())
        external_rows_.insert(var);
}

variable tableau::remove_column(variable var)
{
    assert(!var.is_nil());
    auto ic (columns_.find(var));
    if (ic == columns_.end())
        return var;

    for (const variable& v : ic->second)
        rows_[v].erase(var);

    if (var.is_external())
    {
        external_rows_.erase(var);
        external_parametric_vars_.erase(var);
    }
    columns_.erase(ic);

    return var;
}

linear_expression tableau::remove_row(variable var)
{
    assert(!var.is_nil());
    auto ir (rows_.find(var));
    assert(ir != rows_.end());
    for (auto& p : ir->second.terms())
    {
        auto ic (columns_.find(p.first));
        assert(ic != columns_.end());
        ic->second.erase(var);
        if (ic->second.empty())
        {
            columns_.erase(ic);
            external_parametric_vars_.erase(p.first);
        }
    }

    infeasible_rows_.erase(var);
    if (var.is_external())
    {
        external_rows_.erase(var);
        external_parametric_vars_.erase(var);
    }

    linear_expression result (ir->second);
    rows_.erase(ir);

    return result;
}

// Replace all occurrences of oldVar with expr, and update column cross indices
// oldVar should now be a basic variable
// Uses the Columns data structure and calls SubstituteOut on each
// row that has oldVar in it
// oldVar is leaving the basis, and becoming parametric
void tableau::substitute_out(variable old, const linear_expression& expr)
{
    auto ic (columns_.find(old));
    if (ic == columns_.end())
        return;

    for (auto& v : ic->second)
    {
        auto& row (rows_[v]);
        row.substitute_out(old, expr, v, *this);
        if (v.is_restricted() && row.constant() < 0)
            infeasible_rows_.insert(v);
    }
    columns_.erase(old);
    if (old.is_external())
    {
        if (!columns_[old].empty())
            external_rows_.insert(old);

        external_parametric_vars_.erase(old);
    }
}

bool tableau::is_valid() const
{
    for (auto& r : rows_)
    {
        const auto& clv (r.first);
        if (clv.is_external())
        {
            if (external_rows_.count(clv) == 0)
                return false;
        }

        auto& expr (r.second);
        for (auto& p : expr.terms())
        {
            const variable& v (p.first);
            if (v.is_external())
            {
                if (external_parametric_vars_.count(v) == 0)
                    return false;
            }
        }
    }
    return true;
}

void tableau::note_removed_variable(const variable& v, const variable& subj)
{
    auto& column (columns_[v]);
    auto i (column.find(subj));
    if (i == column.end())
        throw internal_error("note_removed_variable");

    column.erase(i);
    if (column.empty())
    {
        columns_.erase(v);
        external_rows_.erase(v);
        external_parametric_vars_.erase(v);
    }
}

void tableau::note_added_variable(const variable& v, const variable& subj)
{
    columns_[v].insert(subj);
    if (v.is_external() && !is_basic_var(v))
        external_parametric_vars_.insert(v);
}

} // namespace rhea
