
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE rhea
#include <boost/test/unit_test.hpp>

#include <random>
#include <boost/range/algorithm.hpp>

#include "../rhea/simplex_solver.hpp"
#include "../rhea/linear_equation.hpp"
#include "../rhea/iostream.hpp"
#include "../rhea/errors_expl.hpp"

using namespace rhea;

struct point
{
    variable x, y;

    point(double a = 0, double b = 0) : x(a), y(b) { }

    bool operator ==(const std::pair<int,int>& p) const
        { return x.value() == p.first && y.value() == p.second; }
};

namespace std
{
    template <typename a, typename b>
    std::ostream& operator<< (std::ostream& s, const std::pair<a,b>& p)
    {
        return s << p.first << "," << p.second;
    }

    inline
    std::ostream& operator<< (std::ostream& s, const point& p)
    {
        return s << p.x << " " << p.y;
    }
}

BOOST_AUTO_TEST_CASE (strength_test)
{
    BOOST_CHECK(strength::required().is_required());
}

BOOST_AUTO_TEST_CASE (variable_test)
{
    variable x (3.0);
    variable y (x);
    variable z (3.0);

    BOOST_CHECK(variable::nil_var().is_nil());
    BOOST_CHECK(!x.is_nil());
    BOOST_CHECK(!y.is_nil());
    BOOST_CHECK(x.is_float());

    BOOST_CHECK_EQUAL(x.value(), 3);
    BOOST_CHECK_EQUAL(y.value(), 3);

    std::hash<variable> h;
    BOOST_CHECK_EQUAL(h(x), h(y));
    BOOST_CHECK(x.is(y));
    BOOST_CHECK(h(x) != h(z));
    BOOST_CHECK(!x.is(z));

    y.set_value(4);
    BOOST_CHECK_EQUAL(x.value(), 4);

    variable_set s;
    s.insert(x);
    BOOST_CHECK(s.count(x) > 0);
    BOOST_CHECK(s.count(y) > 0);

    s.erase(y);
    BOOST_CHECK(s.empty());
}

BOOST_AUTO_TEST_CASE (linearexpr1_test)
{
    linear_expression expr1 (5);
    BOOST_CHECK_EQUAL(expr1.evaluate(), 5);
    expr1 *= -1;
    BOOST_CHECK_EQUAL(expr1.evaluate(), -5);

    variable x (3.0), y (2.0);
    linear_expression expr2 (x, 2.0, 1.0);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 7);
    BOOST_CHECK_EQUAL((expr2 + 2.0).evaluate(), 9);
    BOOST_CHECK_EQUAL((expr2 - 1.0).evaluate(), 6);

    expr2 += x;
    BOOST_CHECK_EQUAL(expr2.evaluate(), 10);
    expr2 -= x;
    BOOST_CHECK_EQUAL(expr2.evaluate(), 7);

    expr2 += linear_expression::term(y, 5);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 17);

    y.set_value(1);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 12);
    x.set_value(10);
    BOOST_CHECK_EQUAL(expr2.evaluate(), 26);

    expr2 *= -1;
    BOOST_CHECK_EQUAL(expr2.evaluate(), -26);
}

BOOST_AUTO_TEST_CASE (linearexpr2_test)
{
    variable x (3);
    linear_expression test1 (x, 5, 2);
    linear_expression test2 (test1);

    BOOST_CHECK_EQUAL(test1.evaluate(), 17);
    BOOST_CHECK_EQUAL(test1.evaluate(), 17);

    linear_expression test3 (std::move(test1));
    BOOST_CHECK_EQUAL(test3.evaluate(), 17);
}

BOOST_AUTO_TEST_CASE (linearexpr3_test)
{
    variable x (5), y (2);

    linear_expression expr (x * 2 + y - 1);
    BOOST_CHECK_EQUAL(expr.evaluate(), 11);

    x.set_value(4);
    BOOST_CHECK_EQUAL(expr.evaluate(), 9);

    BOOST_CHECK_EQUAL((x + 3).evaluate(), 7);
    BOOST_CHECK_EQUAL((x - 2).evaluate(), 2);
    BOOST_CHECK_EQUAL((x + y).evaluate(), 6);
    BOOST_CHECK_EQUAL((x - y).evaluate(), 2);
}

BOOST_AUTO_TEST_CASE (linear_equation1_test)
{
    variable x (2.0);
    linear_expression expr (x, 4.0, 1.0);
    variable answer (9.0);

    linear_equation eq1 (expr, answer);
    BOOST_CHECK(eq1.is_satisfied());

    linear_expression expr2 (x, 3.0, 3.0);
    linear_equation eq2 (expr, expr2);
    BOOST_CHECK(eq2.is_satisfied());

    linear_equation eq3 (expr, variable(42.0));
    BOOST_CHECK(!eq3.is_satisfied());
}

BOOST_AUTO_TEST_CASE (linear_equation2_test)
{
    variable x (2.0), y (3.0);

    BOOST_CHECK((x == y - 1).is_satisfied());
    BOOST_CHECK(!(x == y).is_satisfied());
    BOOST_CHECK((x * 2 == y + 1).is_satisfied());
    BOOST_CHECK(!(x * 3 == y * 4).is_satisfied());
}

BOOST_AUTO_TEST_CASE (linear_inequality1_test)
{
    variable x (2.0);
    linear_expression expr (x, 4.0, 1.0);
    variable answer (5.0);

    linear_inequality eq1 (answer, relation::leq, expr);
    BOOST_CHECK(eq1.is_satisfied());
    x.set_value(0);
    BOOST_CHECK(!eq1.is_satisfied());
}

BOOST_AUTO_TEST_CASE (linear_inequality2_test)
{
    variable x (2.0), y (3.0);
    BOOST_CHECK((x < y).is_satisfied());
    BOOST_CHECK((x + 1 <= y).is_satisfied());
    BOOST_CHECK((x * 2 + y > 4).is_satisfied());
    BOOST_CHECK((x * 3 >= y * 2).is_satisfied());
    BOOST_CHECK(!(x > y).is_satisfied());
}

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE (simple1_test)
{
    variable x (167);
    variable y (2);

    simplex_solver solver;

    constraint c (new linear_equation(x, linear_expression(y)));
    solver.add_stay(x);
    solver.add_stay(y);
    solver.add_constraint(c);

    BOOST_CHECK(solver.is_valid());
    BOOST_CHECK_EQUAL(x.value(), y.value());
}

BOOST_AUTO_TEST_CASE (simple2_test)
{
    variable x (167);

    simplex_solver solver;

    BOOST_CHECK_THROW( (solver.add_edit_var(x),
                        solver.begin_edit(),
                        solver.suggest_value(x, 100),
                        solver.end_edit()),
                        edit_misuse);
}

BOOST_AUTO_TEST_CASE (constraint1_test)
{
    variable x (0);
    simplex_solver solver;
    solver.add_constraint(new linear_equation(x, 10, strength::weak()));
    BOOST_CHECK_EQUAL(x.value(), 10.0);
}

BOOST_AUTO_TEST_CASE (juststay1_test)
{
    variable x(5), y(10);
    simplex_solver solver;

    solver.add_stay(x).add_stay(y);

    BOOST_CHECK_EQUAL(x.value(), 5);
    BOOST_CHECK_EQUAL(y.value(), 10);
}

BOOST_AUTO_TEST_CASE (editleak1_test)
{
    variable x (0);
    simplex_solver solver;
    solver.add_stay(x);

    BOOST_CHECK_EQUAL(solver.columns().size(), 2);
    BOOST_CHECK_EQUAL(solver.rows().size(), 2);

    solver.add_edit_var(x);
    solver.begin_edit();
    solver.suggest_value(x, 2);

    BOOST_CHECK_EQUAL(solver.columns().size(), 3);
    BOOST_CHECK_EQUAL(solver.rows().size(), 3);

    solver.end_edit();

    BOOST_CHECK_EQUAL(x.value(), 2.0);
    BOOST_CHECK_EQUAL(solver.columns().size(), 2);
    BOOST_CHECK_EQUAL(solver.rows().size(), 2);
}

BOOST_AUTO_TEST_CASE (editleak2_test)
{
    variable x(0), y(0);
    simplex_solver solver;
    solver.add_stay(x).add_stay(y);

    BOOST_CHECK_EQUAL(solver.columns().size(), 4);
    BOOST_CHECK_EQUAL(solver.rows().size(), 3);

    solver.add_edit_var(x).add_edit_var(y);

    solver.begin_edit();
    solver.suggest_value(x, 2);
    solver.suggest_value(y, 4);

    BOOST_CHECK_EQUAL(solver.columns().size(), 6);
    BOOST_CHECK_EQUAL(solver.rows().size(), 5);

    solver.end_edit();

    BOOST_CHECK_EQUAL(x.value(), 2.0);
    BOOST_CHECK_EQUAL(y.value(), 4.0);
    BOOST_CHECK_EQUAL(solver.columns().size(), 4);
    BOOST_CHECK_EQUAL(solver.rows().size(), 3);
}

BOOST_AUTO_TEST_CASE (delete1_test)
{
    variable x (0);
    simplex_solver solver;

    constraint init (x == 100, strength::weak());
    solver.add_constraint(init);
    BOOST_CHECK_EQUAL(x.value(), 100);

    constraint c10 (x <= 10),
               c20 (x <= 20);

    solver.add_constraint(c10).add_constraint(c20);

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

    BOOST_CHECK_EQUAL(solver.columns().size(), 0);
    BOOST_CHECK_EQUAL(solver.rows().size(), 1);
}

BOOST_AUTO_TEST_CASE (delete2_test)
{
    variable x (0), y (0);
    simplex_solver solver;

    solver.add_constraint(x == 100, strength::weak())
          .add_constraint(y == 120, strength::strong());

    BOOST_CHECK_EQUAL(x.value(), 100);
    BOOST_CHECK_EQUAL(y.value(), 120);

    constraint c10 (x <= 10), c20 (x <= 20);

    solver.add_constraint(c10).add_constraint(c20);

    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 20);

    constraint cxy (x * 2 == y);
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

BOOST_AUTO_TEST_CASE (delete3_test)
{
    variable x (0);
    simplex_solver solver;

    solver.add_constraint(x == 100, strength::weak());;

    BOOST_CHECK_EQUAL(x.value(), 100);

    constraint c10 (x <= 10), c10b (x <= 10);

    solver.add_constraint(c10).add_constraint(c10b);

    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10b);
    BOOST_CHECK_EQUAL(x.value(), 100);
}

BOOST_AUTO_TEST_CASE (casso1_test)
{
    variable x (0), y (0);
    simplex_solver solver;

    solver.add_constraint(x <= y)
          .add_constraint(y == x + 3)
          .add_constraint(x == 10.0, strength::weak())
          .add_constraint(y == 10.0, strength::weak());

    BOOST_CHECK(   (x.value() == 10 && y.value() == 13)
                || (x.value() == 7  && y.value() == 10));
}

BOOST_AUTO_TEST_CASE (casso2_test)
{
    variable x (0), y (0);
    simplex_solver solver;

    solver.add_constraints(
    {
        x <= y, y == x + 3, x == 10
    });
    BOOST_CHECK_EQUAL(x.value(), 10);
    BOOST_CHECK_EQUAL(y.value(), 13);
}

BOOST_AUTO_TEST_CASE (inconsistent1_test)
{
    variable x (0);
    simplex_solver solver;

    solver.add_constraint(x == 10);

    BOOST_CHECK_THROW(solver.add_constraint(x == 5),
                      required_failure);

}

BOOST_AUTO_TEST_CASE (inconsistent2_test)
{
    variable x (0);
    simplex_solver solver;

    BOOST_CHECK_THROW(solver.add_constraints({ x >= 10, x <= 5 }),
                      required_failure);

}

BOOST_AUTO_TEST_CASE (inconsistent3_test)
{
    variable v (0), w (0), x (0), y (0);
    simplex_solver solver;

    solver.add_constraints({ v >= 10, w >= v, x >= w, y >= x});
    BOOST_CHECK_THROW(solver.add_constraint(y <= 5), required_failure);
}

BOOST_AUTO_TEST_CASE (inconsistent4_test)
{
    variable v (0), w (0), x (0), y (0);
    simplex_solver solver;

    solver.set_explaining(true);
    solver.add_constraints({ v >= 10, w >= v, x >= w, y >= x});

    try
    {
        solver.add_constraint(y <= 5);
    }
    catch (required_failure_with_explanation& e)
    {
        BOOST_CHECK_EQUAL(e.explanation().size(), 5);
    }
    catch (...)
    {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_CASE (multiedit1_test)
{
    variable x(3), y(-5), w(0), h(0);
    simplex_solver solver;

    solver.add_stay(x).add_stay(y).add_stay(w).add_stay(h);
    solver.add_edit_var(x).add_edit_var(y);
    {
    scoped_edit outer_edit (solver);

    solver.suggest_value(x, 10).suggest_value(y, 20);
    solver.resolve();

    BOOST_CHECK_EQUAL(x.value(), 10);
    BOOST_CHECK_EQUAL(y.value(), 20);
    BOOST_CHECK_EQUAL(w.value(),  0);
    BOOST_CHECK_EQUAL(h.value(),  0);

    solver.add_edit_var(w).add_edit_var(h);
    {
    scoped_edit inner_edit (solver);
    solver.suggest_value(w, 30).suggest_value(h, 40);
    }

    BOOST_CHECK_EQUAL(x.value(), 10);
    BOOST_CHECK_EQUAL(y.value(), 20);
    BOOST_CHECK_EQUAL(w.value(), 30);
    BOOST_CHECK_EQUAL(h.value(), 40);

    solver.suggest_value(x, 50).suggest_value(y, 60);
    }

    BOOST_CHECK_EQUAL(x.value(), 50);
    BOOST_CHECK_EQUAL(y.value(), 60);
    BOOST_CHECK_EQUAL(w.value(), 30);
    BOOST_CHECK_EQUAL(h.value(), 40);
}

BOOST_AUTO_TEST_CASE (multiedit2_test)
{
    variable x(3), y(0), w(0), h(0);
    simplex_solver solver;

    solver.add_stay(x).add_stay(y).add_stay(w).add_stay(h);
    solver.add_edit_var(x).add_edit_var(y);
    {
    scoped_edit outer_edit (solver);

    solver.suggest_value(x, 10).suggest_value(y, 20);
    solver.resolve();

    BOOST_CHECK_EQUAL(x.value(), 10);
    BOOST_CHECK_EQUAL(y.value(), 20);
    BOOST_CHECK_EQUAL(w.value(),  0);
    BOOST_CHECK_EQUAL(h.value(),  0);

    solver.add_edit_var(x).add_edit_var(y)
          .add_edit_var(w).add_edit_var(h);

    {
    scoped_edit inner_edit (solver);
    solver.suggest_value(w, 30).suggest_value(h, 40);
    }

    BOOST_CHECK_EQUAL(x.value(), 10);
    BOOST_CHECK_EQUAL(y.value(), 20);
    BOOST_CHECK_EQUAL(w.value(), 30);
    BOOST_CHECK_EQUAL(h.value(), 40);

    solver.suggest_value(x, 50).suggest_value(y, 60);
    }

    BOOST_CHECK_EQUAL(x.value(), 50);
    BOOST_CHECK_EQUAL(y.value(), 60);
    BOOST_CHECK_EQUAL(w.value(), 30);
    BOOST_CHECK_EQUAL(h.value(), 40);
}

BOOST_AUTO_TEST_CASE (bounds_test)
{
    variable x(1);
    simplex_solver solver;

    solver.add_stay(x).add_bounds(x, 0, 10);

    BOOST_CHECK_EQUAL(x.value(), 1);
    solver.add_edit_var(x);
    solver.begin_edit().suggest_value(x, 20).end_edit();
    BOOST_CHECK_EQUAL(x.value(), 10);
}

BOOST_AUTO_TEST_CASE (bug0_test)
{
    variable x (7), y (8), z(9);
    simplex_solver solver;

    solver.add_stay(x).add_stay(y).add_stay(z);

    solver.add_edit_var(x).add_edit_var(y).add_edit_var(z);
    solver.begin_edit();

    solver.suggest_value(x, 1);
    solver.suggest_value(z, 2);

    solver.remove_edit_var(y);

    solver.suggest_value(x, 3);
    solver.suggest_value(z, 4);

    solver.end_edit();
}

BOOST_AUTO_TEST_CASE (quad_test)
{
    std::vector<point> c { {50, 50}, {50, 250}, {250, 250}, {250, 50} };
    std::vector<point> m (4);
    simplex_solver solver;

    double factor (1);
    for (point& corner : c)
    {
        solver.add_stay(corner.x, strength::weak(), factor);
        solver.add_stay(corner.y, strength::weak(), factor);
        factor *= 2;
    }

    // Midpoint constraints
    for (int i (0); i < 4; ++i)
    {
        int j ((i + 1) % 4);
        solver.add_constraint(m[i].x == (c[i].x + c[j].x) / 2)
              .add_constraint(m[i].y == (c[i].y + c[j].y) / 2);
    }

    // Don't turn inside out
    typedef std::vector<std::pair<int,int>> pairs;
    for (auto a : pairs{{0,2},{0,3},{1,2},{1,3}})
        solver.add_constraint(c[a.first].x + 1 <= c[a.second].x);

    for (auto a : pairs{{0,1},{0,2},{3,1},{3,2}})
        solver.add_constraint(c[a.first].y + 1 <= c[a.second].y);

    // Limits
    for (auto& corner : c)
    {
        solver.add_bounds(corner.x, 0, 300);
        solver.add_bounds(corner.y, 0, 300);
    }

    // Now for the actual tests
    BOOST_CHECK_EQUAL(c[0], std::make_pair(50, 50));
    BOOST_CHECK_EQUAL(m[0], std::make_pair(50, 150));
    BOOST_CHECK_EQUAL(c[1], std::make_pair(50, 250));
    BOOST_CHECK_EQUAL(m[1], std::make_pair(150, 250));
    BOOST_CHECK_EQUAL(c[2], std::make_pair(250, 250));
    BOOST_CHECK_EQUAL(m[2], std::make_pair(250, 150));

    // Move one of the corners
    solver.suggest({{ c[0].x, 100 }});

    BOOST_CHECK_EQUAL(c[0], std::make_pair(100, 50));
    BOOST_CHECK_EQUAL(m[0], std::make_pair(75, 150));
    BOOST_CHECK_EQUAL(c[1], std::make_pair(50, 250));
    BOOST_CHECK_EQUAL(m[1], std::make_pair(150, 250));
    BOOST_CHECK_EQUAL(c[3], std::make_pair(250, 50));
    BOOST_CHECK_EQUAL(m[3], std::make_pair(175, 50));

    // Move one of the midpoints
    solver.suggest({{ m[0].x, 50 }, { m[0].y, 150 }});

    BOOST_CHECK_EQUAL(m[0], std::make_pair(50, 150));
    BOOST_CHECK_EQUAL(c[0], std::make_pair(50, 50));
    BOOST_CHECK_EQUAL(m[0], std::make_pair(50, 150));
    BOOST_CHECK_EQUAL(m[3], std::make_pair(150, 50));
}

