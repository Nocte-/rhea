//---------------------------------------------------------------------------
/// \file   linear_expression.hpp
/// \brief  A linear expression
//
// Copyright 2015, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include "expression.hpp"
#include "variable.hpp"

namespace rhea
{

/** Linear expression.
 * Expressions have the form \f$a_0x_0 + a_1x_1 + \ldots + c\f$, where \f$x_n\f$
 * is a variable, \f$a_n\f$ are non-zero coefficients, and \f$c\f$ is a
 * constant.
 *
 * A \a linear_expression can be built from variables in a natural way:
 *
 * \code

 variable x {3}, y {5};
 auto expr = x * 5 + y + 2;
 expr.evaluate(); // Returns '22'

 * \endcode
 */
class linear_expression : public expression<variable>
{
public:
    linear_expression(double constant = 0.0)
        : expression{constant}
    {
    }

    linear_expression(const variable& v, double coeff = 1.0,
                      double constant = 0.0)
        : expression{v, coeff, constant}
    {
    }

    linear_expression(const expression& copy)
        : expression{copy}
    {
    }

    linear_expression(expression&& m)
        : expression{std::move(m)}
    {
    }

    double evaluate() const
    {
        auto result = constant();
        for (auto&& p : terms())
            result += p.first.value() * p.second;

        return result;
    }
};

//--------------------------------------------------------------------------

inline linear_expression operator*(linear_expression e, double x)
{
    return e *= x;
}

inline linear_expression operator*(double x, linear_expression e)
{
    return e *= x;
}

inline linear_expression operator/(linear_expression e, double x)
{
    return e /= x;
}

inline linear_expression operator*(linear_expression e,
                                   const linear_expression& x)
{
    return e *= x;
}

inline linear_expression operator/(linear_expression e,
                                   const linear_expression& x)
{
    return e /= x;
}

inline linear_expression operator+(linear_expression e,
                                   const linear_expression& x)
{
    return e += x;
}

inline linear_expression operator-(linear_expression e,
                                   const linear_expression& x)
{
    return e -= x;
}

//--------------------------------------------------------------------------

inline linear_expression operator*(const variable& v, double x)
{
    return {v, x};
}

inline linear_expression operator*(const variable& v, int x)
{
    return {v, static_cast<double>(x)};
}

inline linear_expression operator*(double x, const variable& v)
{
    return {v, x};
}

inline linear_expression operator/(const variable& v, double x)
{
    return {v, 1. / x};
}

inline linear_expression operator+(const variable& v, double x)
{
    return {v, 1., x};
}

inline linear_expression operator+(const variable& v, int x)
{
    return {v, 1., static_cast<double>(x)};
}

inline linear_expression operator-(const variable& v, double x)
{
    return {v, 1., -x};
}

inline linear_expression operator-(const variable& v, int x)
{
    return {v, 1., -static_cast<double>(x)};
}

template <typename T>
inline linear_expression operator-(double x, const variable& v)
{
    return {v, -1., x};
}

template <typename T>
inline linear_expression operator+(const variable& v, const variable& w)
{
    return linear_expression{v} += w;
}

template <typename T>
inline linear_expression operator-(const variable& v, const variable& w)
{
    return linear_expression{v} -= w;
}

} // namespace rhea
