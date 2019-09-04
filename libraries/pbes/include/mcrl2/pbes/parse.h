// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/parse.h
/// \brief Parser for pbes expressions.

#ifndef MCRL2_PBES_PARSE_H
#define MCRL2_PBES_PARSE_H

#include "mcrl2/core/parse.h"
#include "mcrl2/core/parser_utility.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/detail/parse_substitution.h"
#include "mcrl2/data/parse.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/typecheck.h"
#include "mcrl2/utilities/text_utility.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace mcrl2
{

namespace pbes_system
{

struct untyped_pbes
{
  data::untyped_data_specification dataspec;
  data::variable_list global_variables;
  std::vector<pbes_equation> equations;
  propositional_variable_instantiation initial_state;

  pbes construct_pbes() const
  {
    pbes result;
    result.data() = dataspec.construct_data_specification();
    result.global_variables() = std::set<data::variable>(global_variables.begin(), global_variables.end());
    result.equations() = equations;
    result.initial_state() = initial_state;
    return result;
  }
};

namespace detail
{

struct pbes_actions: public data::detail::data_specification_actions
{
  explicit pbes_actions(const core::parser& parser_)
    : data::detail::data_specification_actions(parser_)
  {}

  pbes_system::pbes_expression parse_PbesExpr(const core::parse_node& node) const
  {
    if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "DataValExpr")) { return parse_DataValExpr(node.child(0)); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "DataExpr")) { return parse_DataExpr(node.child(0)); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "true")) { return pbes_system::true_(); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "false")) { return pbes_system::false_(); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "forall") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "PbesExpr")) { return pbes_system::forall(parse_VarsDeclList(node.child(1)), parse_PbesExpr(node.child(3))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "exists") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "PbesExpr")) { return pbes_system::exists(parse_VarsDeclList(node.child(1)), parse_PbesExpr(node.child(3))); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "!") && (symbol_name(node.child(1)) == "PbesExpr")) { return pbes_system::not_(parse_PbesExpr(node.child(1))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "PbesExpr") && (node.child(1).string() == "=>") && (symbol_name(node.child(2)) == "PbesExpr")) { return pbes_system::imp(parse_PbesExpr(node.child(0)), parse_PbesExpr(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "PbesExpr") && (node.child(1).string() == "&&") && (symbol_name(node.child(2)) == "PbesExpr")) { return pbes_system::and_(parse_PbesExpr(node.child(0)), parse_PbesExpr(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "PbesExpr") && (node.child(1).string() == "||") && (symbol_name(node.child(2)) == "PbesExpr")) { return pbes_system::or_(parse_PbesExpr(node.child(0)), parse_PbesExpr(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "(") && (symbol_name(node.child(1)) == "PbesExpr") && (symbol_name(node.child(2)) == ")")) { return parse_PbesExpr(node.child(1)); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "PropVarInst")) { return parse_PropVarInst(node.child(0)); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "Id")) { return data::untyped_data_parameter(parse_Id(node.child(0)), parse_DataExprList(node.child(1))); }
    throw core::parse_node_unexpected_exception(m_parser, node);
  }

  pbes_system::propositional_variable parse_PropVarDecl(const core::parse_node& node) const
  {
    return pbes_system::propositional_variable(parse_Id(node.child(0)), parse_VarsDeclList(node.child(1)));
  }

  pbes_system::propositional_variable_instantiation parse_PropVarInst(const core::parse_node& node) const
  {
    return pbes_system::propositional_variable_instantiation(parse_Id(node.child(0)), parse_DataExprList(node.child(1)));
  }

  pbes_system::propositional_variable_instantiation parse_PbesInit(const core::parse_node& node) const
  {
    return parse_PropVarInst(node.child(1));
  }

  pbes_system::fixpoint_symbol parse_FixedPointOperator(const core::parse_node& node) const
  {
    if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "mu")) { return fixpoint_symbol::mu(); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "nu")) { return fixpoint_symbol::nu(); }
    throw core::parse_node_unexpected_exception(m_parser, node);
  }

  pbes_equation parse_PbesEqnDecl(const core::parse_node& node) const
  {
    return pbes_equation(parse_FixedPointOperator(node.child(0)), parse_PropVarDecl(node.child(1)), parse_PbesExpr(node.child(3)));
  }

  std::vector<pbes_equation> parse_PbesEqnDeclList(const core::parse_node& node) const
  {
    return parse_vector<pbes_equation>(node, "PbesEqnDecl", [&](const core::parse_node& node) { return parse_PbesEqnDecl(node); });
  }

  std::vector<pbes_equation> parse_PbesEqnSpec(const core::parse_node& node) const
  {
    return parse_PbesEqnDeclList(node.child(1));
  }

  untyped_pbes parse_PbesSpec(const core::parse_node& node) const
  {
    untyped_pbes result;
    result.dataspec = parse_DataSpec(node.child(0));
    result.global_variables = parse_GlobVarSpec(node.child(1));
    result.equations = parse_PbesEqnSpec(node.child(2));
    result.initial_state = parse_PbesInit(node.child(3));
    return result;
  }
};

inline
pbes_expression parse_pbes_expression_new(const std::string& text)
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn, core::detail::syntax_error_fn);
  unsigned int start_symbol_index = p.start_symbol_index("PbesExpr");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  core::warn_and_or(node);
  pbes_expression result = pbes_actions(p).parse_PbesExpr(node);
  return result;
}

inline
untyped_pbes parse_pbes_new(const std::string& text)
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn, core::detail::syntax_error_fn);
  unsigned int start_symbol_index = p.start_symbol_index("PbesSpec");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  core::warn_and_or(node);
  untyped_pbes result = pbes_actions(p).parse_PbesSpec(node);
  return result;
}

inline
void complete_pbes(pbes& x)
{
  typecheck_pbes(x);
  pbes_system::translate_user_notation(x);
  complete_data_specification(x);
}

} // namespace detail

inline
pbes parse_pbes(std::istream& in)
{
  std::string text = utilities::read_text(in);
  pbes result = detail::parse_pbes_new(text).construct_pbes();
  detail::complete_pbes(result);
  return result;
}

/// \brief Reads a PBES from an input stream.
/// \param from An input stream
/// \param result A PBES
/// \return The input stream
inline
std::istream& operator>>(std::istream& from, pbes& result)
{
  result = parse_pbes(from);
  return from;
}

inline
pbes parse_pbes(const std::string& text)
{
  std::istringstream in(text);
  return parse_pbes(in);
}

template <typename VariableContainer>
propositional_variable parse_propositional_variable(const std::string& text,
                                                    const VariableContainer& variables,
                                                    const data::data_specification& dataspec = data::data_specification()
                                                   )
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn, core::detail::syntax_error_fn);
  unsigned int start_symbol_index = p.start_symbol_index("PropVarDecl");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  propositional_variable result = detail::pbes_actions(p).parse_PropVarDecl(node);
  return typecheck_propositional_variable(result, variables, dataspec);
}

/** \brief     Parse a pbes expression.
 *  Throws an exception if something went wrong.
 *  \param[in] text A string containing a pbes expression.
 *  \param[in] variables A sequence of data variables that may appear in x.
 *  \param[in] propositional_variables A sequence of propositional variables that may appear in x.
 *  \param[in] dataspec A data specification.
 *  \param[in] type_check If true the parsed input is also typechecked.
 *  \return    The parsed PBES expression.
 **/
template <typename VariableContainer, typename PropositionalVariableContainer>
pbes_expression parse_pbes_expression(const std::string& text,
                                      const data::data_specification& dataspec,
                                      const VariableContainer& variables,
                                      const PropositionalVariableContainer& propositional_variables,
                                      bool type_check = true,
                                      bool translate_user_notation = true,
                                      bool normalize_sorts = true
                                     )
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn, core::detail::syntax_error_fn);
  unsigned int start_symbol_index = p.start_symbol_index("PbesExpr");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  core::warn_and_or(node);
  pbes_expression x = detail::pbes_actions(p).parse_PbesExpr(node);
  if (type_check)
  {
    x = pbes_system::typecheck_pbes_expression(x, variables, propositional_variables, dataspec);
  }
  if (translate_user_notation)
  {
    x = pbes_system::translate_user_notation(x);
  }
  if (normalize_sorts)
  {
    x = pbes_system::normalize_sorts(x, dataspec);
  }
  return x;
}

/** \brief     Parse a pbes expression.
 *  Throws an exception if something went wrong.
 *  \param[in] text A string containing a pbes expression.
 *  \param[in] pbesspec A PBES used as context.
 *  \param[in] variables A sequence of data variables that may appear in x.
 *  \param[in] type_check If true the parsed input is also typechecked.
 *  \return    The parsed PBES expression.
 **/
template <typename VariableContainer>
pbes_expression parse_pbes_expression(const std::string& text,
                                      const pbes& pbesspec,
                                      const VariableContainer& variables,
                                      bool type_check = true,
                                      bool translate_user_notation = true,
                                      bool normalize_sorts = true
                                     )
{
  std::vector<propositional_variable> propositional_variables;
  for (const pbes_equation& eqn: pbesspec.equations())
  {
    propositional_variables.push_back(eqn.variable());
  }
  return parse_pbes_expression(text, pbesspec.data(), variables, propositional_variables, type_check, translate_user_notation, normalize_sorts);
}

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_PARSE_H
