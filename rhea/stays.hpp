//---------------------------------------------------------------------------
/// \file   stays.hpp
/// \brief  Try to keep edit variables at their current value.
//
// Copyright 2016, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <unordered_map>

#include "constraint.hpp"
#include "simplex_solver.hpp"
#include "strength.hpp"
#include "variable.hpp"

namespace rhea
{

/** Register edit variables as stays.
 * Doing so will make underconstrained variables stay at their old value. */
class stays
{
    struct item
    {
        constraint c;
        double v;
    };

public:
    stays(simplex_solver& solver)
        : solver_{solver}
    {
    }

    void add(const variable& v)
    {
        if (stays_.count(v) == 0)
        {
            auto c = (v == v.value() | strength::weak());
            stays_[v] = item{c, v.value()};
            solver_.add_constraint(c);
        }
    }

    void remove(const variable& v)
    {
        auto found = stays_.find(v);
        if (found != stays_.end())
        {
            solver_.remove_constraint(found->second.c);
            stays_.erase(found);
        }
    }

    /** Update the stay constraints.
     * You must call this after suggesting new values for variables. */
    void update()
    {
        for (auto&& kvp : stays_)
        {
            auto& v = kvp.first;
            auto& it = kvp.second;

            if (v.value() != it.v)
            {
                solver_.remove_constraint(it.c);
                it.v = v.value();
                it.c = (v == it.v | strength::weak());
                solver_.add_constraint(it.c);
            }
        }
    }

private:
    simplex_solver& solver_;
    std::unordered_map<variable, item> stays_;
};

} // namespace rhea
