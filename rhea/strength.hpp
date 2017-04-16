//---------------------------------------------------------------------------
/// \file   strength.hpp
/// \brief  The strength of a constraint
//
// Copyright 2015-2017, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

namespace rhea
{

/** Every constraint has a strength that determines where it sits in the
 ** hierarchy; strong constraints are always satisfied in preference
 ** to weaker ones.
 *
 * Since we want to be able to express preferences as well as requirements
 * in the constraint system, we need a specification for how conflicting
 * preferences are to be traded off.  In a constraint hierarchy each
 * constraint has a strength.  The 'required' strength is special, in that
 * required constraints must be satisfied.  The other strengths
 * all label non-required constraints.  A constraint of a given strength
 * completely dominates any constraint with a weaker strength.  In the
 * hierarchy theory, a comparator is used to compare different possible
 * solutions to the constraints and select among them.
 *
 * Within this framework a number of variations are possible.  One decision
 * is whether we only compare solutions on a constraint-by-constraint basis
 * (a \e local comparator), or whether we take some aggregate measure of the
 * unsatisfied constraints of a given strength (a \e global comparator).  A
 * second choice is whether we are concerned only whether a constraint is
 * satisfied or not (a \e predicate comparator), or whether we also want to
 * know how nearly satisfied it is (a \e metric comparator).
 *
 * Constraints whose domain is a metric space, for example the reals, can
 * have an associated error function.  The error in satisfying a constraint
 * is zero if and only if the constraint is satisfied, and becomes larger
 * the less nearly satisfied it is.  For a given collection of constraints,
 * Cassowary finds a \e locally-error-better or a \e weighted-sum-better
 * solution.
 */
class strength
{
public:
    operator double() const { return weight_; }

    /** Constraints with this strength must be satisfied.
     *  Used by default for constraints provided by the programmer. */
    static strength required() { return {1000. * 1000. * 1000.}; }

    /** Strong constraint, the default for edit constraints. */
    static strength strong() { return {1000. * 1000.}; }
    /** Medium strength constraint. */
    static strength medium() { return {1000.}; }
    /** Weak constraint, the default strength for stay constraints. */
    static strength weak() { return {1.}; }

    /** Strong constraint strength with weight factor.
     * \param weight A 1..1000 weight factor */
    static strength strong(double weight)
    {
        check_(weight);
        return {weight * 1000. * 1000.};
    }

    /** Medium constraint strength with weight factor.
    * \param weight A 1..1000 weight factor */
    static strength medium(double weight)
    {
        check_(weight);
        return {weight * 1000.};
    }

    /** Weak constraint strength with weight factor.
    * \param weight A 1..10000 weight factor */
    static strength weak(double weight)
    {
        check_(weight);
        return {weight};
    }

    /** Check if this strength signals a required constraint. */
    bool is_required() const { return weight_ == required().weight_; }

    bool operator==(const strength& c) const { return weight_ == c.weight_; }
    bool operator!=(const strength& c) const { return weight_ != c.weight_; }
    bool operator<=(const strength& c) const { return weight_ <= c.weight_; }
    bool operator<(const strength& c) const { return weight_ < c.weight_; }
    bool operator>=(const strength& c) const { return weight_ >= c.weight_; }
    bool operator>(const strength& c) const { return weight_ > c.weight_; }

    strength operator-() const { return {-weight_}; }

private:
    strength(double w)
        : weight_{w}
    {
    }

    static void check_(double v)
    {
        if (v < 1.0 || v >= 1000.)
            throw bad_weight{};
    }

private:
    double weight_;
};

} // namespace rhea
