//---------------------------------------------------------------------------
/// \file   relation.hpp
/// \brief  Relation between two sides in an inequality
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

namespace rhea {

class relation
{
public:
    // enum setup so additive inverse flips the direction of the inequality
    typedef enum
    {
        eq = 0, neq = 100, leq = 2, geq = -2, lt = 3, gt = -3
    }
    comp_t;

public:
    relation(comp_t type = eq) : type_(type) { }

    comp_t type() const
        { return type_; }

    relation reverse_inequality() const
        { return type_ == neq ? type_ : comp_t(-(int)type_); }

    std::string to_string() const
    {
        switch (type_)
        {
            case eq:    return "==";
            case neq:   return "!=";
            case leq:   return "<=";
            case geq:   return ">=";
            case lt:    return "<";
            case gt:    return ">";
            default: assert(false); return "?";
        }
    }

    bool operator== (comp_t c) const
         { return type_ == c; }

private:
    comp_t type_;
};

} // namespace rhea
