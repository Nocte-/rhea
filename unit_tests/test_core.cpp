
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE rhea
#include <boost/test/unit_test.hpp>

#include <random>
#include <boost/range/algorithm.hpp>

#include "../src/simplex_solver.hpp"
#include "../src/linear_equation.hpp"

using namespace rhea;

BOOST_AUTO_TEST_CASE (strength_test)
{
    BOOST_CHECK(strength::required().is_required());
}

BOOST_AUTO_TEST_CASE (variable_test)
{
    variable x (3.0);
    variable y (x);
    variable z (3.0);

    BOOST_CHECK(variable::nil().is_nil());
    BOOST_CHECK(!x.is_nil());
    BOOST_CHECK(!y.is_nil());
    BOOST_CHECK(x.is_float());

    BOOST_CHECK_EQUAL(x.value(), 3);
    BOOST_CHECK_EQUAL(y.value(), 3);

    std::hash<variable> h;
    BOOST_CHECK_EQUAL(h(x), h(y));
    BOOST_CHECK(x == y);
    BOOST_CHECK(h(x) != h(z));
    BOOST_CHECK(x != z);

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

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE (simple1_test)
{
    variable x ("x", 167);
    variable y ("y", 2);

    simplex_solver solver;

    constraint_ref c (new linear_equation(x, linear_expression(y)));
    solver.add_var(x);
    solver.add_var(y);
    solver.add_constraint(c);

    BOOST_CHECK(solver.is_valid());
    BOOST_CHECK_EQUAL(x.value(), y.value());
}

BOOST_AUTO_TEST_CASE (simple2_test)
{
    variable x ("x", 167);

    simplex_solver solver;

    BOOST_CHECK_THROW( (solver.add_edit_var(x),
                        solver.begin_edit(),
                        solver.suggest_value(x, 100),
                        solver.end_edit()),
                        edit_misuse);
}

BOOST_AUTO_TEST_CASE (constraint1_test)
{
    variable x;
    simplex_solver solver;
    solver.add_constraint(new linear_equation(x, 10, strength::weak()));
    BOOST_CHECK_EQUAL(x.value(), 10.0);
}

BOOST_AUTO_TEST_CASE (juststay1_test)
{
    variable x(5.0), y(10.0);
    simplex_solver solver;
    solver.add_stay(x).add_stay(y);

    BOOST_CHECK_EQUAL(x.value(), 5.0);
    BOOST_CHECK_EQUAL(y.value(), 10.0);
}

BOOST_AUTO_TEST_CASE (delete1_test)
{
    variable x ("x");
    simplex_solver solver;

    solver.add_constraint(new linear_equation(x, 100, strength::weak()));
    BOOST_CHECK_EQUAL(x.value(), 100.0);

    constraint_ref c10 (std::make_shared<linear_inequality>(x, relation::leq, 10.0)),
                   c20 (std::make_shared<linear_inequality>(x, relation::leq, 20.0));

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
}

BOOST_AUTO_TEST_CASE (delete2_test)
{
    variable x ("x"), y ("y");
    simplex_solver solver;

    solver.add_constraint(new linear_equation(x, 100, strength::weak()))
          .add_constraint(new linear_equation(y, 120, strength::strong()));

    BOOST_CHECK_EQUAL(x.value(), 100);
    BOOST_CHECK_EQUAL(y.value(), 120);

    constraint_ref c10 (std::make_shared<linear_inequality>(x, relation::leq, 10.0)),
                   c20 (std::make_shared<linear_inequality>(x, relation::leq, 20.0));

    solver.add_constraint(c10).add_constraint(c20);

    BOOST_CHECK_EQUAL(x.value(), 10);
    solver.remove_constraint(c10);
    BOOST_CHECK_EQUAL(x.value(), 20);

    constraint_ref cxy (std::make_shared<linear_equation>(linear_expression(x) * 2, y));
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

BOOST_AUTO_TEST_CASE (casso1_test)
{
    variable x ("x"), y ("y");
    simplex_solver solver;

    solver.add_constraint(new linear_inequality(x, relation::leq, y))
          .add_constraint(new linear_equation(y, linear_expression(x) + 3))
          .add_constraint(new linear_equation(x, 10.0, strength::weak()))
          .add_constraint(new linear_equation(y, 10.0, strength::weak()));

    BOOST_CHECK(   (x.value() == 10 && y.value() == 13)
                || (x.value() == 7  && y.value() == 10));
}

BOOST_AUTO_TEST_CASE (inconsistent1_test)
{
    variable x ("x");
    simplex_solver solver;

    BOOST_CHECK_THROW(solver.add_constraint(new linear_equation(x, 10))
                            .add_constraint(new linear_equation(x, 5)),
                      required_failure);

}

BOOST_AUTO_TEST_CASE (inconsistent2_test)
{
    variable x ("x");
    simplex_solver solver;

    BOOST_CHECK_THROW(solver.add_constraint(new linear_inequality(x, relation::geq, 10))
                            .add_constraint(new linear_inequality(x, relation::leq, 5)),
                      required_failure);

}

BOOST_AUTO_TEST_CASE (inconsistent3_test)
{
    variable v ("v"), w ("w"), x ("x"), y ("y");
    simplex_solver solver;

    BOOST_CHECK_THROW(solver.add_constraint(new linear_inequality(v, relation::geq, 10))
                            .add_constraint(new linear_inequality(w, relation::geq, v))
                            .add_constraint(new linear_inequality(x, relation::geq, w))
                            .add_constraint(new linear_inequality(y, relation::geq, x))
                            .add_constraint(new linear_inequality(y, relation::geq, 5)),
                      required_failure);
}

BOOST_AUTO_TEST_CASE (multiedit1_test)
{
    variable x, y, w, h;
    simplex_solver solver;

    solver.add_stay(x).add_stay(y).add_stay(w).add_stay(h);

    solver.add_edit_var(x).add_edit_var(y);
    solver.begin_edit();
    solver.suggest_value(x, 10).suggest_value(y, 20);
    solver.end_edit();

    BOOST_CHECK_EQUAL(x.value(), 10);
    BOOST_CHECK_EQUAL(y.value(), 20);
    BOOST_CHECK_EQUAL(w.value(),  0);
    BOOST_CHECK_EQUAL(h.value(),  0);

    solver.add_edit_var(w).add_edit_var(h);
    solver.begin_edit();
    solver.suggest_value(w, 30).suggest_value(h, 40);
    solver.end_edit();

    BOOST_CHECK_EQUAL(x.value(), 10);
    BOOST_CHECK_EQUAL(y.value(), 20);
    BOOST_CHECK_EQUAL(w.value(), 30);
    BOOST_CHECK_EQUAL(h.value(), 40);

    solver.begin_edit();
    solver.suggest_value(w, 50).suggest_value(h, 60);
    solver.end_edit();

    BOOST_CHECK_EQUAL(x.value(), 50);
    BOOST_CHECK_EQUAL(y.value(), 60);
    BOOST_CHECK_EQUAL(w.value(), 30);
    BOOST_CHECK_EQUAL(h.value(), 40);
}

BOOST_AUTO_TEST_CASE (bug0_test)
{
    variable x ("x",7), y ("y",8), z("z",9);
    simplex_solver solver;

    solver.add_var(x).add_var(y).add_var(z);

    solver.add_edit_var(x).add_edit_var(y).add_edit_var(z);
    solver.begin_edit();

    solver.suggest_value(x, 1);
    solver.suggest_value(z, 2);

    solver.remove_edit_var(y);

    solver.suggest_value(x, 3);
    solver.suggest_value(z, 4);

    solver.end_edit();
}

