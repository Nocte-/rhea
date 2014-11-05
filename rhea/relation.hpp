//---------------------------------------------------------------------------
/// \file   relation.hpp
/// \brief  Relation between two sides in an equation or inequality
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

namespace rhea
{

/** An (in)equality relation. */
class relation
{
public:
    /** This enumeration is set up in such a way that additive inverse flips
     ** the direction of the inequality. */
    typedef enum {
        /** Equal to. */
        eq = 0,
        /** Not equal to. */
        neq = 100,
        /** Less than or equal to. */
        leq = 2,
        /** Greater than or equal to. */
        geq = -2,
        /** Less than. */
        lt = 3,
        /** Greater than. */
        gt = -3
    } comp_t;

public:
    relation(comp_t type = eq)
        : type_(type)
    {
    }

    comp_t type() const { return type_; }

    relation reverse_inequality() const
    {
        return type_ == neq ? type_ : comp_t(-(int)type_);
    }

    std::string to_string() const
    {
        switch (type_) {
        case eq:
            return "==";
        case neq:
            return "!=";
        case leq:
            return "<=";
        case geq:
            return ">=";
        case lt:
            return "<";
        case gt:
            return ">";
        default:
            assert(false);
            return "?";
        }
    }

    bool operator==(comp_t c) const { return type_ == c; }

private:
    comp_t type_;
};

} // namespace rhea
