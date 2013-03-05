//---------------------------------------------------------------------------
/// \file   errors_expl.hpp
/// \brief  Adds an explanation to the required_failure exception class
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include "constraint.hpp"
#include "errors.hpp"

namespace rhea {

/** One of the required constraints cannot be satisfied.
 *  This exception extends required_failure with a list of the constraints
 *  that were involved.  Dropping one or more of the constraints, or
 *  lowering their priority, will usually solve the problem. */
class required_failure_with_explanation : public required_failure
{
public:
    required_failure_with_explanation(constraint_list cl)
        : expl_(std::move(cl))
    { }

    virtual ~required_failure_with_explanation() throw() { }

    const constraint_list& explanation() const { return expl_; }

    void add (constraint c) { expl_.emplace_back(c); }

protected:
    constraint_list expl_;
};

} // namespace rhea


