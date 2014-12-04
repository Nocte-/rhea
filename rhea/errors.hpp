//---------------------------------------------------------------------------
/// \file   errors.hpp
/// \brief  Exception classes
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <string>
#include <stdexcept>

namespace rhea
{

class variable;

/** Base class for all Rhea exceptions. */
class error : public std::exception
{
public:
    virtual ~error() noexcept {}

    // LCOV_EXCL_START
    virtual const char* what() const noexcept { return "unspecified error"; }
    // LCOV_EXCL_STOP
};

/** Signals an internal inconsistency in the solver. */
class internal_error : public error
{
    std::string msg;

public:
    internal_error(std::string m)
        : msg(m)
    {
    }

    virtual ~internal_error() noexcept {}

    virtual const char* what() const noexcept { return msg.c_str(); }
};

/** Thrown whenever the usual ordering of setting up edit constraints is
 ** not observed.
 * The usual order is:
 * - simplex_solver::add_edit_var()
 * - simplex_solver::begin_edit()
 * - simplex_solver::suggest_value()
 * - simplex_solver::end_edit()
 *
 * This is done automatically by simplex_solver::suggest(). */
class edit_misuse : public error
{
    const variable* var_;

public:
    edit_misuse()
        : var_{nullptr}
    {
    }
    edit_misuse(const rhea::variable& v)
        : var_{&v}
    {
    }
    virtual ~edit_misuse() noexcept {}

    virtual const char* what() const noexcept
    {
        return "edit protocol usage violation";
    }

    const variable& var() const { return *var_; }
};

/** The constraints are too difficult to solve. */
class too_difficult : public error
{
    std::string msg;

public:
    too_difficult() {}

    too_difficult(std::string m)
        : msg(m)
    {
    }

    virtual ~too_difficult() noexcept {}

    virtual const char* what() const noexcept
    {
        return msg.empty() ? "the constraints are too difficult to solve"
                           : msg.c_str();
    }
};

/** Read-only constraints are not allowed by this particular solver
 ** implementation. */
class readonly_not_allowed : public too_difficult
{
public:
    virtual ~readonly_not_allowed() noexcept {}

    virtual const char* what() const noexcept
    {
        return "the read-only annotation is not permitted by the solver";
    }
};

/** Cyclic dependencies between constraints are not allowed. */
class cycle_not_allowed : public too_difficult
{
public:
    virtual ~cycle_not_allowed() noexcept {}

    virtual const char* what() const noexcept
    {
        return "a cyclic constraint graph is not permitted by the solver";
    }
};

/** This solver cannot handle strict inequalities.
 * Strict inequalities are \f$<\f$ and \f$>\f$.  They can be solved in the
 * finite domain, but the simplex solver can only deal with \f$\leq\f$ and
 * \f$\geq\f$. */
class strict_inequality_not_allowed : public too_difficult
{
public:
    virtual ~strict_inequality_not_allowed() noexcept {}

    virtual const char* what() const noexcept
    {
        return "the strict inequality is not permitted by the solver";
    }
};

/** One of the required constraints cannot be satisfied. */
class required_failure : public error
{
public:
    virtual ~required_failure() noexcept {}

    virtual const char* what() const noexcept
    {
        return "a required constraint cannot be satisfied";
    }
};

/** Not enough stay constraints were specified to give specific values
 ** to every variable. */
class not_enough_stays : public error
{
public:
    virtual ~not_enough_stays() noexcept {}

    virtual const char* what() const noexcept
    {
        return "there are not enough stays to give specific values to every "
               "variable";
    }
};

/** The resulting expression would be nonlinear.
 * This usually happens when multiplying two expressions that have the
 * same variable in them, resulting in a quadratic expression. */
class nonlinear_expression : public error
{
public:
    virtual ~nonlinear_expression() noexcept {}

    virtual const char* what() const noexcept
    {
        return "the resulting expression would be nonlinear";
    }
};

/** The application tried to remove a constraint that doesn't exist in
 ** the solver. */
class constraint_not_found : public error
{
public:
    virtual ~constraint_not_found() noexcept {}

    virtual const char* what() const noexcept
    {
        return "tried to remove a constraint that was never added";
    }
};

/** The application tried to remove a row that doesn't exist. */
class row_not_found : public error
{
public:
    virtual ~row_not_found() noexcept {}

    virtual const char* what() const noexcept { return "row does not exist"; }
};

} // namespace rhea
