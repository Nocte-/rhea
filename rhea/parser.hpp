//---------------------------------------------------------------------------
/// \file   parser.hpp
/// \brief  Construct a constraint from a string
//
// Copyright 2012-2014, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------

#include <unordered_map>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "constraint.hpp"
#include "linear_equation.hpp"
#include "linear_inequality.hpp"

namespace rhea
{

typedef boost::spirit::qi::symbols<char, variable> var_map;

template <typename Iterator>
struct constraint_grammar : boost::spirit::qi::grammar<Iterator, constraint()>
{
    typedef boost::spirit::ascii::space_type space_type;

    constraint_grammar(var_map& vars)
        : constraint_grammar::base_type{constr}
    {
        using namespace boost::spirit;
        namespace phx = boost::phoenix;
        using qi::double_;
        using qi::_val;

        constr = lineq[_val = phx::construct<constraint>(_1)]
                 | linineq[_val = phx::construct<constraint>(_1)];

        lineq = (expr >> '='
                 >> expr)[_val = phx::construct<linear_equation>(_1, _2)];

        linineq
            = (expr >> "<=" >> expr)[_val = phx::construct<linear_inequality>(
                                         _1, relation::leq, _2)]
              | (expr >> ">="
                 >> expr)[_val = phx::construct<linear_inequality>(
                              _1, relation::geq, _2)];

        expr = double_[_val = _1]
               //| lexeme[raw[(ascii::alpha >> *(ascii::alnum | '_'))]] [
               // vars.add, _val = vars[_1] ]
               | vars[_val = _1] | (expr >> '+' >> expr)[_val = _1 + _2]
               | (expr >> '-' >> expr)[_val = _1 - _2]
               | (expr >> '*' >> expr)[_val = _1 * _2]
               | (expr >> '/' >> expr)[_val = _1 / _2];

        constr.name("constraint");
        lineq.name("linear equation");
        linineq.name("linear inequality");
        expr.name("expression");

        qi::on_error<qi::fail>(
            constr, std::cout << phx::val("Error! Expecting ")
                              << _4 // what failed?
                              << phx::val(" here: \"")
                              << phx::construct<std::string>(
                                     _3, _2) // iterators to error-pos, end
                              << phx::val("\"") << std::endl);
    }

    boost::spirit::qi::rule<Iterator, constraint()> constr;
    boost::spirit::qi::rule<Iterator, linear_equation()> lineq;
    boost::spirit::qi::rule<Iterator, linear_inequality()> linineq;
    boost::spirit::qi::rule<Iterator, linear_expression()> expr;
};
}
