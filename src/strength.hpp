//---------------------------------------------------------------------------
/// \file   strength.hpp
/// \brief  A named symbolic weight
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

#include <string>
#include "symbolic_weight.hpp"

namespace rhea {

class strength
{
public:
    strength(std::string name, symbolic_weight weight)
        : name_(name)
        , weight_(weight)
    { }

    strength(std::string name, double a, double b, double c)
        : name_(name)
        , weight_(a, b, c)
    { }

    virtual ~strength() { }

    static strength required()
        { return strength("required", 1000, 1000, 1000); }

    static strength strong()
        { return strength("strong", 1, 0, 0); }

    static strength medium()
        { return strength("medium", 0, 1, 0); }

    static strength weak()
        { return strength("weak", 0, 0, 1); }

    virtual bool is_required() const
        { return weight_ == required().weight_; }

    const symbolic_weight& weight() const
        { return weight_; }

    bool operator==(const strength& c) const { return weight_ == c.weight_; }
    bool operator!=(const strength& c) const { return weight_ != c.weight_; }
    bool operator<=(const strength& c) const { return weight_ <= c.weight_; }
    bool operator< (const strength& c) const { return weight_ <  c.weight_; }
    bool operator>=(const strength& c) const { return weight_ >= c.weight_; }
    bool operator> (const strength& c) const { return weight_ >  c.weight_; }

private:
    std::string     name_;
    symbolic_weight weight_;
};

} // namespace rhea
