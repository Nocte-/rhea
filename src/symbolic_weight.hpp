//---------------------------------------------------------------------------
/// \file   symbolic_weight.hpp
/// \brief  A 3-tuple weight for constraints
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

#include <cstddef>
#include <vector>

namespace rhea {

class symbolic_weight
{
public:
    symbolic_weight(int levels = 3, double value = 0.0);
    symbolic_weight(double w1, double w2, double w3);
    symbolic_weight(std::vector<double> weights);

    static symbolic_weight zero();

    symbolic_weight& negate();
    symbolic_weight& operator *= (double n);
    symbolic_weight& operator /= (double n);
    symbolic_weight& operator += (const symbolic_weight& n);
    symbolic_weight& operator -= (const symbolic_weight& n);

    bool operator<  (const symbolic_weight& comp) const;
    bool operator<= (const symbolic_weight& comp) const;
    bool operator== (const symbolic_weight& comp) const;
    bool operator!= (const symbolic_weight& comp) const;
    bool operator>  (const symbolic_weight& comp) const;
    bool operator>= (const symbolic_weight& comp) const;

    bool is_negative() const;

    double as_double() const
    {
        double sum(0), factor(1), multiplier(1000000);
        for (auto i (values_.rbegin()); i != values_.rend(); ++i)
        {
            sum += *i * factor;
            factor *= multiplier;
        }
        return sum;
    }

    size_t levels() const
        { return values_.size(); }

    bool approx(const symbolic_weight& w, double n)
    {
        auto i (w.values_.begin());
        if (!approx(*i, n))
            return false;

        for (++i; i != w.values_.end(); ++i)
        {
            if (!approx(*i, 0))
                return false;
        }
        return true;
    }

    bool approx(const symbolic_weight& w1, const symbolic_weight& w2)
    {
        auto i1 (w1.values_.begin()), i2 (w2.values_.begin());
        for (; i1 != w1.values_.end() && i2 != w2.values_.end(); ++i1, ++i2)
        {
            if (!approx(*i1, *i2))
                return false;
        }
        return i1 == w1.values_.end() && i2 == w2.values_.end();
    }

private:
    std::vector<double> values_;
};

inline symbolic_weight operator* (symbolic_weight w, double n)
{
    return w *= n;
}

inline symbolic_weight operator/ (symbolic_weight w, double n)
{
    return w /= n;
}

inline symbolic_weight operator+ (symbolic_weight w, const symbolic_weight& n)
{
    return w += n;
}

inline symbolic_weight operator- (symbolic_weight w, const symbolic_weight& n)
{
    return w -= n;
}

} // namespace rhea
