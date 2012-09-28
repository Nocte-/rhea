//---------------------------------------------------------------------------
/// \file   errors.hpp
/// \brief  Exception classes
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
#include <stdexcept>

namespace rhea {

class error : public std::exception
{
public:
    virtual ~error() throw() { }

    virtual const char* what() const throw()
    {
        return "unspecified error";
    }
};

class internal_error : public error
{
    std::string msg;

public:
    internal_error(std::string m) : msg(m) { }
    virtual ~internal_error() throw() { }

    virtual const char* what() const throw()
        { return msg.c_str(); }
};

class bad_resolve : public error
{
public:
    virtual ~bad_resolve() throw() { }

    virtual const char* what() const throw()
        { return "number of resolve values did not match number of edit vars"; }
};

class edit_misuse : public error
{
    std::string varname_;

public:
    edit_misuse() { }
    edit_misuse(std::string name) : varname_(name) { }
    virtual ~edit_misuse() throw() { }

    virtual const char* what() const throw()
        { return "edit protocol usage violation"; }

    const std::string& variable_name() const { return varname_; }
};

class too_difficult : public error
{
    std::string msg;

public:
    too_difficult() { }
    too_difficult(std::string m) : msg(m) { }
    virtual ~too_difficult() throw() { }

    virtual const char* what() const throw()
        { return msg.empty() ? "the constraints are too difficult to solve" : msg.c_str(); }
};

class readonly_not_allowed : public too_difficult
{
public:
    virtual ~readonly_not_allowed() throw() { }

    virtual const char* what() const throw()
        { return "the read-only annotation is not permitted by the solver"; }
};

class cycle_not_allowed : public too_difficult
{
public:
    virtual ~cycle_not_allowed() throw() { }

    virtual const char* what() const throw()
        { return "a cyclic constraint graph is not permitted by the solver"; }
};

class strict_inequality_not_allowed : public too_difficult
{
public:
    virtual ~strict_inequality_not_allowed() throw() { }

    virtual const char* what() const throw()
        { return "the strict inequality is not permitted by the solver"; }
};

class required_failure : public error
{
public:
    virtual ~required_failure() throw() { }

    virtual const char* what() const throw()
        { return "a required constraint cannot be satisfied"; }
};

class not_enough_stays : public error
{
public:
    virtual ~not_enough_stays() throw() { }

    virtual const char* what() const throw()
        { return "there are not enough stays to give specific values to every variable"; }
};

class nonlinear_expression : public error
{
public:
    virtual ~nonlinear_expression() throw() { }

    virtual const char* what() const throw()
        { return "the resulting expression would be nonlinear"; }
};

class constraint_not_found : public error
{
public:
    virtual ~constraint_not_found() throw() { }

    virtual const char* what() const throw()
        { return "tried to remove a constraint that was never added"; }
};

class row_not_found : public error
{
public:
    virtual ~row_not_found() throw() { }

    virtual const char* what() const throw()
        { return "row does not exist"; }
};

} // namespace rhea


