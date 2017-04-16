
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE rhea
#include <boost/test/unit_test.hpp>

#include <random>
#include <boost/range/algorithm.hpp>

#include "rhea/simplex_solver.hpp"
#include "rhea/stays.hpp"
#include "rhea/iostream.hpp"
#include "rhea/linear_expression.hpp"
#include "rhea/link_variable.hpp"

using namespace rhea;

struct point
{
    variable x, y;

    point(double a = 0, double b = 0)
        : x(a)
        , y(b)
    {
    }

    bool operator==(const std::pair<int, int>& p) const
    {
        return x.value() == p.first && y.value() == p.second;
    }
};

// LCOV_EXCL_START
namespace std
{
template <typename a, typename b>
std::ostream& operator<<(std::ostream& s, const std::pair<a, b>& p)
{
    return s << p.first << "," << p.second;
}

inline std::ostream& operator<<(std::ostream& s, const point& p)
{
    return s << p.x << " " << p.y;
}
}
// LCOV_EXCL_STOP

BOOST_AUTO_TEST_CASE(strength_test)
{
    BOOST_CHECK(strength::required().is_required());
    BOOST_CHECK(!strength::strong().is_required());
    BOOST_CHECK(!strength::medium().is_required());
    BOOST_CHECK(!strength::weak().is_required());

    BOOST_CHECK(strength::required() > strength::strong());
    BOOST_CHECK(strength::strong() > strength::medium());
    BOOST_CHECK(strength::medium() > strength::weak());

    BOOST_CHECK(strength::required() > strength::strong(999));
    BOOST_CHECK(strength::strong(100) > strength::strong(10));
    BOOST_CHECK(strength::strong(1) > strength::medium(999));
    BOOST_CHECK(strength::medium(1) > strength::weak(999));
}

BOOST_AUTO_TEST_CASE(variable_test)
{
    variable a;
    variable m(variable::nil_var()), n(variable::nil_var());
    variable x(3.0);
    variable y(x);
    variable z(3.0);

    BOOST_CHECK(n.is_nil());
    n = x;
    a = y;
    BOOST_CHECK(m.is_nil());
    BOOST_CHECK(!n.is_nil());
    BOOST_CHECK(!x.is_nil());
    BOOST_CHECK(!y.is_nil());

    BOOST_CHECK_EQUAL(x.value(), 3);
    BOOST_CHECK_EQUAL(x.int_value(), 3);
    BOOST_CHECK_EQUAL(y.value(), 3);
    BOOST_CHECK_EQUAL(a.value(), 3);

    std::hash<variable> h;
    BOOST_CHECK_EQUAL(h(x), h(y));
    BOOST_CHECK(x.is(y));
    BOOST_CHECK(h(x) != h(z));
    BOOST_CHECK(!x.is(z));
    BOOST_CHECK(a.is(x));

    y.set_value(3.7);
    BOOST_CHECK_EQUAL(n.value(), 3.7);
    BOOST_CHECK_EQUAL(x.value(), 3.7);
    BOOST_CHECK_EQUAL(x.int_value(), 4);

    y.set_value(-3.7);
    BOOST_CHECK_EQUAL(x.int_value(), -4);
}

BOOST_AUTO_TEST_CASE(variable_stream_test)
{
    variable x{3.0};
    std::stringstream s;
    s << x;
    BOOST_CHECK_EQUAL(s.str(), "{var:3}");

    float y = 6.f;
    variable yv{y, linked()};
    std::stringstream s2;
    s2 << yv;
    BOOST_CHECK_EQUAL(s2.str(), "{link:6}");

    int z = 9;
    variable zv{z, linked()};
    std::stringstream s3;
    s3 << zv;
    BOOST_CHECK_EQUAL(s3.str(), "{link:9}");
}

BOOST_AUTO_TEST_CASE(variable_nil_stream_test)
{
    auto x = variable::nil_var();
    std::stringstream s;
    s << x;
    BOOST_CHECK_EQUAL(s.str(), "NIL");
}

BOOST_AUTO_TEST_CASE(constraint_stream_test)
{
    std::stringstream s;
    s << constraint{variable{1} + 42 == variable{2}};
    BOOST_CHECK(s.str() == "{var:2}*-1 + {var:1}*1 + 42 == 0 | required"
                || s.str() == "{var:1}*1 + {var:2}*-1 + 42 == 0 | required");

    std::stringstream t;
    t << constraint{variable{5} <= 43};
    BOOST_CHECK_EQUAL(t.str(), "{var:5}*1 + -43 <= 0 | required");

    std::stringstream u;
    u << constraint{variable{6} >= 43};
    BOOST_CHECK_EQUAL(u.str(), "{var:6}*1 + -43 >= 0 | required");
}

BOOST_AUTO_TEST_CASE(strength_stream_test)
{
    std::stringstream s;
    s << strength::required();
    BOOST_CHECK_EQUAL(s.str(), "required");
    s.str("");
    s << strength::strong();
    BOOST_CHECK_EQUAL(s.str(), "strong");
    s.str("");
    s << strength::medium();
    BOOST_CHECK_EQUAL(s.str(), "medium");
    s.str("");
    s << strength::weak();
    BOOST_CHECK_EQUAL(s.str(), "weak");
}

BOOST_AUTO_TEST_CASE(linearexpr1_test)
{
    linear_expression expr1(5);
    BOOST_CHECK_EQUAL(expr1.evaluate(), 5);
    expr1 *= -1;
    BOOST_CHECK_EQUAL(expr1.evaluate(), -5);

    variable x(3.0), y(2.0);
    linear_expression expr2(x, 2.0, 1.0);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 7);
    BOOST_CHECK_EQUAL((expr2 + 2.0).evaluate(), 9);
    BOOST_CHECK_EQUAL((expr2 - 1.0).evaluate(), 6);

    expr2 += x;
    BOOST_CHECK_EQUAL(expr2.evaluate(), 10);
    expr2 -= x;
    BOOST_CHECK_EQUAL(expr2.evaluate(), 7);

    expr2 += y * 5;
    BOOST_CHECK_EQUAL(expr2.evaluate(), 17);

    y.set_value(1);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 12);
    x.set_value(10);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 26);

    expr2 *= -1;
    BOOST_CHECK_EQUAL(expr2.evaluate(), -26);

    expr2 /= 2;
    BOOST_CHECK_EQUAL(expr2.evaluate(), -13);

    expr1 *= expr2;
    BOOST_CHECK_EQUAL(expr1.evaluate(), 65);
}

BOOST_AUTO_TEST_CASE(linearexpr2_test)
{
    variable x(3);
    linear_expression test1(x, 5.0, 2.0);
    linear_expression test2(test1);

    BOOST_CHECK_EQUAL(test1.evaluate(), 17);
    BOOST_CHECK_EQUAL(test1.evaluate(), 17);

    linear_expression test3(std::move(test1));
    BOOST_CHECK_EQUAL(test3.evaluate(), 17);
}

BOOST_AUTO_TEST_CASE(linearexpr3_test)
{
    variable x(5), y(2);

    linear_expression expr(x * 2 + y - 1);
    BOOST_CHECK_EQUAL(expr.evaluate(), 11);

    x.set_value(4);
    BOOST_CHECK_EQUAL(expr.evaluate(), 9);

    BOOST_CHECK_EQUAL((x + 3).evaluate(), 7);
    BOOST_CHECK_EQUAL((x - 2).evaluate(), 2);
    BOOST_CHECK_EQUAL((x + y).evaluate(), 6);
    BOOST_CHECK_EQUAL((x - y).evaluate(), 2);
}

BOOST_AUTO_TEST_CASE(linear_equation_test)
{
    variable x(2.0), y(3.0);

    BOOST_CHECK((x == y - 1).is_satisfied());
    BOOST_CHECK(!(x == y).is_satisfied());
    BOOST_CHECK((x * 2 == y + 1).is_satisfied());
    BOOST_CHECK(!(x * 3 == y * 4).is_satisfied());
}

BOOST_AUTO_TEST_CASE(linear_inequality_test)
{
    variable x(2.0), y(3.0);
    BOOST_CHECK((x <= y).is_satisfied());
    BOOST_CHECK((x + 1 <= y).is_satisfied());
    BOOST_CHECK((x * 2 + y >= 4).is_satisfied());
    BOOST_CHECK((x * 3 >= y * 2).is_satisfied());
    BOOST_CHECK(!(x >= y).is_satisfied());
}

BOOST_AUTO_TEST_CASE(constraint_map_test)
{
    variable x;
    constraint c1{x == 1};
    std::unordered_map<constraint, int> map;
    map[c1] = 5;

    constraint c2 = c1;
    BOOST_CHECK(c1 == c2);

    std::hash<constraint> hasher;
    BOOST_CHECK(hasher(c1) == hasher(c2));
    BOOST_CHECK(map.count(c1) == 1);
    BOOST_CHECK(map.count(c2) == 1);
}

BOOST_AUTO_TEST_CASE(substitute_out_test)
{
    variable x, y, z;
    auto c1 = linear_expression{x * 4 + y * 2 + z};
    c1.substitute_out(y, z + 3);

    BOOST_CHECK_EQUAL(c1.constant(), 6);
    BOOST_CHECK_EQUAL(c1.coefficient(x), 4);
    BOOST_CHECK_EQUAL(c1.coefficient(y), 0);
    BOOST_CHECK_EQUAL(c1.coefficient(z), 3);
}

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(constraint1_test)
{
    variable x(0);
    simplex_solver solver;
    solver.add_constraint(x == 10);
    BOOST_CHECK_EQUAL(x.value(), 10.0);
}


BOOST_AUTO_TEST_CASE(juststaylink1_test)
{
    float ox = 5.0f, oy = 10.0f;
    variable x{ox, linked()}, y{oy, linked()};
    simplex_solver solver;

    BOOST_CHECK_EQUAL(x.value(), 5);
    BOOST_CHECK_EQUAL(ox, 5);
    BOOST_CHECK_EQUAL(oy, 10);

    solver.suggest({{x, 6}, {y, 7}});

    BOOST_CHECK_EQUAL(x.value(), 6);
    BOOST_CHECK_EQUAL(x.int_value(), 6);
    BOOST_CHECK_EQUAL(ox, 6);
    BOOST_CHECK_EQUAL(oy, 7);
}

BOOST_AUTO_TEST_CASE(juststaylink2_test)
{
    int ox = 5, oy = 10;
    variable x(ox, linked()), y(oy, linked());
    simplex_solver solver;

    BOOST_CHECK_EQUAL(ox, 5);
    BOOST_CHECK_EQUAL(oy, 10);

    solver.suggest({{x, 6.2}, {y, 7.4}});

    BOOST_CHECK_EQUAL(ox, 6);
    BOOST_CHECK_EQUAL(oy, 7);

    BOOST_CHECK_EQUAL(x.value(), 6.2);
    BOOST_CHECK_EQUAL(x.int_value(), 6);
}

BOOST_AUTO_TEST_CASE(juststaylink3_test)
{
    double ox = 5, oy = 10;
    variable x([&](double v) { ox = v; }, 5.0);
    variable y([&](double v) { oy = v; }, 10.0);
    simplex_solver solver;

    BOOST_CHECK_EQUAL(ox, 5);
    BOOST_CHECK_EQUAL(oy, 10);

    solver.suggest({{x, 6}, {y, 7}});

    BOOST_CHECK_EQUAL(ox, 6);
    BOOST_CHECK_EQUAL(oy, 7);
}

BOOST_AUTO_TEST_CASE(justedit1_test) // Issue 37
{
    variable x(5);
    simplex_solver solver;
    solver.suggest(x, 6);
    BOOST_CHECK_EQUAL(x.value(), 6);
}

BOOST_AUTO_TEST_CASE(delete1_test)
{
    variable x(0);
    simplex_solver solver;

    constraint init(x == 100, strength::weak());
    solver.add_constraint(init);
    BOOST_CHECK_EQUAL(x.value(), 100);

    constraint c10(x <= 10), c20(x <= 20);

    solver.add_constraint(c10);
    solver.add_constraint(c20);
    BOOST_CHECK_EQUAL(x.value(), 10.0);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 20.0);
    solver.remove_constraint(c20);
    BOOST_CHECK_EQUAL(x.value(), 100.0);
    solver.add_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 10.0);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 100.0);
    solver.remove_constraint(init);
}

BOOST_AUTO_TEST_CASE(delete2_test)
{
    variable x(0), y(0);
    simplex_solver solver;

    solver.add_constraints(
        {x == 100 | strength::weak(), y == 120 | strength::strong()});

    BOOST_CHECK_EQUAL(x.value(), 100);
    BOOST_CHECK_EQUAL(y.value(), 120);

    constraint c10(x <= 10), c20(x <= 20);

    solver.add_constraint(c10);
    solver.add_constraint(c20);
    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 20);

    constraint cxy(x * 2 == y);
    solver.add_constraint(cxy);
    BOOST_CHECK_EQUAL(x.value(), 20);
    BOOST_CHECK_EQUAL(y.value(), 40);

    solver.remove_constraint(c20);
    BOOST_CHECK_EQUAL(x.value(), 60);
    BOOST_CHECK_EQUAL(y.value(), 120);

    solver.remove_constraint(cxy);
    BOOST_CHECK_EQUAL(x.value(), 100);
    BOOST_CHECK_EQUAL(y.value(), 120);
}

BOOST_AUTO_TEST_CASE(delete3_test)
{
    variable x(0);
    simplex_solver solver;

    solver.add_constraint(x == 100 | strength::weak());
    BOOST_CHECK_EQUAL(x.value(), 100);

    constraint c10(x <= 10), c10b(x <= 10);

    solver.add_constraints({c10, c10b});
    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10);

    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10b);

    BOOST_CHECK_EQUAL(x.value(), 100);
}

BOOST_AUTO_TEST_CASE(set_constant1_test)
{
    variable x(0);
    simplex_solver solver;
    auto cn = solver.add_constraint(x == 100);
    BOOST_CHECK_EQUAL(x.value(), 100);
    solver.set_constant(cn, 110);
    BOOST_CHECK_EQUAL(x.value(), 110);

    solver.set_constant(cn, 150);
    BOOST_CHECK_EQUAL(x.value(), 150);

    solver.set_constant(cn, -25);
    BOOST_CHECK_EQUAL(x.value(), -25);
}

BOOST_AUTO_TEST_CASE(set_constant2_test)
{
    variable x;
    simplex_solver solver;
    auto cn = solver.add_constraint(x == 100 | strength::medium());
    BOOST_CHECK_EQUAL(x.value(), 100);
    solver.set_constant(cn, 110);
    BOOST_CHECK_EQUAL(x.value(), 110);

    solver.set_constant(cn, 150);
    BOOST_CHECK_EQUAL(x.value(), 150);

    solver.set_constant(cn, -25);
    BOOST_CHECK_EQUAL(x.value(), -25);
}

BOOST_AUTO_TEST_CASE(set_constant3_test)
{
    variable x;
    simplex_solver solver;
    auto cn = solver.add_constraint(x >= 100);
    BOOST_CHECK_EQUAL(x.value(), 100);
    solver.set_constant(cn, 110);
    BOOST_CHECK_EQUAL(x.value(), 110);

    solver.set_constant(cn, 150);
    BOOST_CHECK_EQUAL(x.value(), 150);

    solver.set_constant(cn, -25);
    BOOST_CHECK_EQUAL(x.value(), -25);
}

BOOST_AUTO_TEST_CASE(set_constant4_test)
{
    variable x;
    simplex_solver solver;
    auto cn = solver.add_constraint(x <= 100);
    BOOST_CHECK_EQUAL(x.value(), 100);
    solver.set_constant(cn, 50);
    BOOST_CHECK_EQUAL(x.value(), 50);

    solver.set_constant(cn, 150);
    BOOST_CHECK_EQUAL(x.value(), 150);

    solver.set_constant(cn, -25);
    BOOST_CHECK_EQUAL(x.value(), -25);
}

BOOST_AUTO_TEST_CASE(set_constant5_test)
{
    variable x;
    simplex_solver solver;
    auto cn = solver.add_constraint(x >= 100 | strength::medium());
    BOOST_CHECK_EQUAL(x.value(), 100);
    solver.set_constant(cn, 110);
    BOOST_CHECK_EQUAL(x.value(), 110);

    solver.set_constant(cn, 150);
    BOOST_CHECK_EQUAL(x.value(), 150);
}

BOOST_AUTO_TEST_CASE(set_constant6_test)
{
    variable x;
    simplex_solver solver;
    auto cn = solver.add_constraint(x <= 100 | strength::medium());
    BOOST_CHECK_EQUAL(x.value(), 100);
    solver.set_constant(cn, 50);
    BOOST_CHECK_EQUAL(x.value(), 50);

    solver.set_constant(cn, -10);
    BOOST_CHECK_EQUAL(x.value(), -10);
}

BOOST_AUTO_TEST_CASE(casso1_test)
{
    variable x, y;
    simplex_solver solver;

    solver.add_constraints({
        x <= y,
        y == x + 3,
        (x == 10.0) | strength::weak(),
        (y == 10.0) | strength::weak()});

    BOOST_CHECK((x.value() == 10 && y.value() == 13)
                || (x.value() == 7 && y.value() == 10));
}

BOOST_AUTO_TEST_CASE(casso2_test)
{
    variable x, y;
    simplex_solver solver;

    solver.add_constraints({x <= y, y == x + 3, x == 10});

    BOOST_CHECK_EQUAL(x.value(), 10);
    BOOST_CHECK_EQUAL(y.value(), 13);
}

BOOST_AUTO_TEST_CASE(inconsistent1_test)
{
    variable x;
    simplex_solver solver;

    solver.add_constraint(x == 10);

    BOOST_CHECK_THROW(solver.add_constraint(x == 5), required_failure);
}

BOOST_AUTO_TEST_CASE(inconsistent2_test)
{
    variable x;
    simplex_solver solver;

    BOOST_CHECK_THROW(solver.add_constraints({x >= 10, x <= 5}),
                      required_failure);
}

BOOST_AUTO_TEST_CASE(inconsistent3_test)
{
    variable v, w, x, y;
    simplex_solver solver;

    solver.add_constraints({v >= 10, w >= v, x >= w, y >= x});
    BOOST_CHECK_THROW(solver.add_constraint(y <= 5), required_failure);
}

BOOST_AUTO_TEST_CASE(bug0_test)
{
    variable x, y, z;
    simplex_solver solver;

    solver.add_edit_vars({x, y, z});
    solver.suggest_value(x, 1);
    solver.suggest_value(z, 2);
    solver.remove_edit_var(y);
    solver.suggest_value(x, 3);
    solver.suggest_value(z, 4);

    BOOST_CHECK(solver.has_edit_var(x));
    BOOST_CHECK(!solver.has_edit_var(y));

    solver.update_external_variables();
    BOOST_CHECK_EQUAL(x.value(), 3);
}

BOOST_AUTO_TEST_CASE(bad_strength) // issue 18
{
    variable v(0);
    simplex_solver solver;
    BOOST_CHECK_THROW(solver.add_edit_var(v, strength::strong(0)), bad_weight);
    BOOST_CHECK_THROW(solver.add_edit_var(v, strength::required()),
                      bad_required_strength);
}

BOOST_AUTO_TEST_CASE(bug_16)
{
    variable a(1), b(2);
    simplex_solver solver;

    solver.add_constraints({a == b});
    solver.suggest(a, 3);

    BOOST_CHECK_EQUAL(a.value(), 3);
    BOOST_CHECK_EQUAL(b.value(), 3);
}

BOOST_AUTO_TEST_CASE(bug_16b)
{
    simplex_solver solver;
    variable a, b, c;

    solver.add_constraints({a == 10, b == c});
    solver.suggest(c, 100);

    BOOST_CHECK_EQUAL(a.value(), 10);
    BOOST_CHECK_EQUAL(b.value(), 100);
    BOOST_CHECK_EQUAL(c.value(), 100);

    solver.suggest(c, 90);

    BOOST_CHECK_EQUAL(a.value(), 10);
    BOOST_CHECK_EQUAL(b.value(), 90);
    BOOST_CHECK_EQUAL(c.value(), 90);
}

BOOST_AUTO_TEST_CASE(nonlinear) // issue 26
{
    variable x, y;
    simplex_solver solver;

    BOOST_CHECK_THROW(solver.add_constraint(x == 5 / y), nonlinear_expression);
    BOOST_CHECK_THROW(solver.add_constraint(x == y * y), nonlinear_expression);

    linear_expression const2{2};
    BOOST_CHECK_NO_THROW(solver.add_constraint(x == y / const2));
}

BOOST_AUTO_TEST_CASE(stays1)
{
    simplex_solver solver;
    stays st{solver};
    variable a, b;

    solver.add_constraints({a >= b, a >= 0, a <= 10});
    st.add(a);

    solver.suggest(b, 3);
    st.update();
    BOOST_CHECK_EQUAL(a.value(), 3);
    BOOST_CHECK_EQUAL(b.value(), 3);

    solver.suggest(b, 1);
    st.update();
    BOOST_CHECK_EQUAL(a.value(), 3);
    BOOST_CHECK_EQUAL(b.value(), 1);

    solver.suggest(b, 5);
    st.update();
    BOOST_CHECK_EQUAL(a.value(), 5);
    BOOST_CHECK_EQUAL(b.value(), 5);

    solver.suggest(b, 2);
    st.update();
    BOOST_CHECK_EQUAL(a.value(), 5);
    BOOST_CHECK_EQUAL(b.value(), 2);
}

/*

BOOST_AUTO_TEST_CASE(change_strength_test) // issue 33
{
    variable x;
    simplex_solver solver;

    constraint c1{x == 1, strength::weak()};
    constraint c2{x == 2, strength::medium()};
    solver.add_constraints({c1, c2});
    BOOST_CHECK_EQUAL(x.value(), 2);

    solver.change_strength(c1, strength::strong());
    BOOST_CHECK_EQUAL(x.value(), 1);
}

BOOST_AUTO_TEST_CASE(change_weight_test) // issue 33
{
    variable x;
    simplex_solver solver;

    auto c1 = x == 1 | strength::strong(1);
    auto c2 = x == 2 | strength::strong(2);
    solver.add_constraints({c1, c2});
    BOOST_CHECK_EQUAL(x.value(), 2);

    solver.change_weight(c1, 3);
    BOOST_CHECK_EQUAL(x.value(), 1);
}

*/
