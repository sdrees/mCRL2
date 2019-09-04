// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/parse.h
/// \brief add your file description here.

#ifndef MCRL2_MODAL_FORMULA_PARSE_H
#define MCRL2_MODAL_FORMULA_PARSE_H

#include "mcrl2/core/parser_utility.h"
#include "mcrl2/data/merge_data_specifications.h"
#include "mcrl2/lps/parse.h"
#include "mcrl2/modal_formula/has_name_clashes.h"
#include "mcrl2/modal_formula/resolve_name_clashes.h"
#include "mcrl2/modal_formula/state_formula.h"
#include "mcrl2/modal_formula/state_formula_specification.h"
#include "mcrl2/modal_formula/translate_regular_formulas.h"
#include "mcrl2/modal_formula/translate_user_notation.h"
#include "mcrl2/modal_formula/typecheck.h"
#include "mcrl2/process/merge_action_specifications.h"
#include <iostream>

namespace mcrl2
{

namespace action_formulas
{

namespace detail
{

struct action_formula_actions: public lps::detail::multi_action_actions
{
  explicit action_formula_actions(const core::parser& parser_)
    : lps::detail::multi_action_actions(parser_)
  {}

  action_formulas::action_formula parse_ActFrm(const core::parse_node& node) const
  {
    if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "MultAct")) { return process::untyped_multi_action(parse_ActionList(node.child(0))); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "DataValExpr")) { return parse_DataValExpr(node.child(0)); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "DataExpr")) { return parse_DataExpr(node.child(0)); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "true")) { return action_formulas::true_(); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "false")) { return action_formulas::false_(); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "!") && (symbol_name(node.child(1)) == "ActFrm")) { return action_formulas::not_(parse_ActFrm(node.child(1))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "ActFrm") && (node.child(1).string() == "=>") && (symbol_name(node.child(2)) == "ActFrm")) { return action_formulas::imp(parse_ActFrm(node.child(0)), parse_ActFrm(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "ActFrm") && (node.child(1).string() == "&&") && (symbol_name(node.child(2)) == "ActFrm")) { return action_formulas::and_(parse_ActFrm(node.child(0)), parse_ActFrm(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "ActFrm") && (node.child(1).string() == "||") && (symbol_name(node.child(2)) == "ActFrm")) { return action_formulas::or_(parse_ActFrm(node.child(0)), parse_ActFrm(node.child(2))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "forall") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "ActFrm")) { return action_formulas::forall(parse_VarsDeclList(node.child(1)), parse_ActFrm(node.child(3))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "exists") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "ActFrm")) { return action_formulas::exists(parse_VarsDeclList(node.child(1)), parse_ActFrm(node.child(3))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "ActFrm") && (node.child(1).string() == "@") && (symbol_name(node.child(2)) == "DataExpr")) { return action_formulas::at(parse_ActFrm(node.child(0)), parse_DataExpr(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "(") && (symbol_name(node.child(1)) == "ActFrm") && (symbol_name(node.child(2)) == ")")) { return parse_ActFrm(node.child(1)); }
    throw core::parse_node_unexpected_exception(m_parser, node);
  }
};

inline
action_formula parse_action_formula(const std::string& text)
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn, core::detail::syntax_error_fn);
  unsigned int start_symbol_index = p.start_symbol_index("ActFrm");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  core::warn_and_or(node);
  action_formula result = action_formula_actions(p).parse_ActFrm(node);
  return result;
}

} // namespace detail

template <typename ActionLabelContainer = std::vector<state_formulas::variable>, typename VariableContainer = std::vector<data::variable> >
action_formula parse_action_formula(const std::string& text,
                                    const data::data_specification& dataspec,
                                    const VariableContainer& variables,
                                    const ActionLabelContainer& actions
                                   )
{
  action_formula x = detail::parse_action_formula(text);
  x = action_formulas::typecheck_action_formula(x, dataspec, variables, actions);
  x = action_formulas::translate_user_notation(x);
  return x;
}

inline
action_formula parse_action_formula(const std::string& text, const lps::specification& lpsspec)
{
  return parse_action_formula(text, lpsspec.data(), lpsspec.global_variables(), lpsspec.action_labels());
}

} // namespace action_formulas

namespace regular_formulas
{

namespace detail
{

struct regular_formula_actions: public action_formulas::detail::action_formula_actions
{
  explicit regular_formula_actions(const core::parser& parser_)
    : action_formulas::detail::action_formula_actions(parser_)
  {}

  regular_formulas::regular_formula parse_RegFrm(const core::parse_node& node) const
  {
    if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "ActFrm")) { return parse_ActFrm(node.child(0)); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "(") && (symbol_name(node.child(1)) == "RegFrm") && (symbol_name(node.child(2)) == ")")) { return parse_RegFrm(node.child(1)); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "RegFrm") && (symbol_name(node.child(1)) == "*")) { return trans_or_nil(parse_RegFrm(node.child(0))); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "RegFrm") && (symbol_name(node.child(1)) == "+")) { return trans(parse_RegFrm(node.child(0))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "RegFrm") && (node.child(1).string() == ".") && (symbol_name(node.child(2)) == "RegFrm")) { return untyped_regular_formula(core::identifier_string("."), parse_RegFrm(node.child(0)), parse_RegFrm(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "RegFrm") && (node.child(1).string() == "+") && (symbol_name(node.child(2)) == "RegFrm")) { return untyped_regular_formula(core::identifier_string("+"), parse_RegFrm(node.child(0)), parse_RegFrm(node.child(2))); }
    throw core::parse_node_unexpected_exception(m_parser, node);
  }
};

inline
regular_formula parse_regular_formula(const std::string& text)
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn, core::detail::syntax_error_fn);
  unsigned int start_symbol_index = p.start_symbol_index("RegFrm");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  regular_formula result = regular_formula_actions(p).parse_RegFrm(node);
  return result;
}

} // namespace detail

template <typename ActionLabelContainer = std::vector<state_formulas::variable>, typename VariableContainer = std::vector<data::variable> >
regular_formula parse_regular_formula(const std::string& text,
                                      const data::data_specification& dataspec,
                                      const VariableContainer& variables,
                                      const ActionLabelContainer& actions
                                     )
{
  regular_formula x = detail::parse_regular_formula(text);
  x = regular_formulas::typecheck_regular_formula(x, dataspec, variables, actions);
  x = regular_formulas::translate_user_notation(x);
  return x;
}

inline
regular_formula parse_regular_formula(const std::string& text, const lps::specification& lpsspec)
{
  return parse_regular_formula(text, lpsspec.data(), lpsspec.global_variables(), lpsspec.action_labels());
}

} // namespace regular_formulas

namespace state_formulas
{

namespace detail
{

/// \brief Prints a warning if formula contains an action that is not used in lpsspec.
inline
void check_actions(const state_formulas::state_formula& formula, const lps::specification& lpsspec)
{
  std::set<process::action_label> used_lps_actions = lps::find_action_labels(lpsspec.process());
  std::set<process::action_label> used_state_formula_actions = state_formulas::find_action_labels(formula);
  std::set<process::action_label> diff = utilities::detail::set_difference(used_state_formula_actions, used_lps_actions);
  if (!diff.empty())
  {
    mCRL2log(log::warning) << "Warning: the modal formula contains an action " << *diff.begin() << " that does not appear in the LPS!" << std::endl;
  }
}

struct untyped_state_formula_specification: public data::untyped_data_specification
{
  process::action_label_list action_labels;
  state_formula formula;

  state_formula_specification construct_state_formula_specification()
  {
    state_formula_specification result;
    result.data() = construct_data_specification();
    result.action_labels() = action_labels;
    result.formula() = formula;
    return result;
  }
};

struct state_formula_actions: public regular_formulas::detail::regular_formula_actions
{
  explicit state_formula_actions(const core::parser& parser_)
    : regular_formulas::detail::regular_formula_actions(parser_)
  {}

  state_formula make_delay(const core::parse_node& node) const
  {
    if (node.child(0))
    {
      return delay_timed(parse_DataExpr(node.child(0).child(1)));
    }
    else
    {
      return delay();
    }
  }

  state_formula make_yaled(const core::parse_node& node) const
  {
    if (node.child(0))
    {
      return yaled_timed(parse_DataExpr(node.child(0).child(1)));
    }
    else
    {
      return yaled();
    }
  }

  data::assignment parse_StateVarAssignment(const core::parse_node& node) const
  {
    return data::assignment(data::variable(parse_Id(node.child(0)), parse_SortExpr(node.child(2))), parse_DataExpr(node.child(4)));
  }

  data::assignment_list parse_StateVarAssignmentList(const core::parse_node& node) const
  {
    return parse_list<data::assignment>(node, "StateVarAssignment", [&](const core::parse_node& node) { return parse_StateVarAssignment(node); });
  }

  state_formulas::state_formula parse_StateFrm(const core::parse_node& node) const
  {
    if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "DataValExpr")) { return parse_DataValExpr(node.child(0)); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "DataExpr")) { return parse_DataExpr(node.child(0)); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "true")) { return state_formulas::true_(); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "false")) { return state_formulas::false_(); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "!") && (symbol_name(node.child(1)) == "StateFrm")) { return state_formulas::not_(parse_StateFrm(node.child(1))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "StateFrm") && (node.child(1).string() == "=>") && (symbol_name(node.child(2)) == "StateFrm")) { return state_formulas::imp(parse_StateFrm(node.child(0)), parse_StateFrm(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "StateFrm") && (node.child(1).string() == "&&") && (symbol_name(node.child(2)) == "StateFrm")) { return state_formulas::and_(parse_StateFrm(node.child(0)), parse_StateFrm(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "StateFrm") && (node.child(1).string() == "||") && (symbol_name(node.child(2)) == "StateFrm")) { return state_formulas::or_(parse_StateFrm(node.child(0)), parse_StateFrm(node.child(2))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "forall") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "StateFrm")) { return state_formulas::forall(parse_VarsDeclList(node.child(1)), parse_StateFrm(node.child(3))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "exists") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "StateFrm")) { return state_formulas::exists(parse_VarsDeclList(node.child(1)), parse_StateFrm(node.child(3))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "[") && (symbol_name(node.child(1)) == "RegFrm") && (symbol_name(node.child(2)) == "]") && (symbol_name(node.child(3)) == "StateFrm")) { return state_formulas::must(parse_RegFrm(node.child(1)), parse_StateFrm(node.child(3))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "<") && (symbol_name(node.child(1)) == "RegFrm") && (symbol_name(node.child(2)) == ">") && (symbol_name(node.child(3)) == "StateFrm")) { return state_formulas::may(parse_RegFrm(node.child(1)), parse_StateFrm(node.child(3))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "mu") && (symbol_name(node.child(1)) == "StateVarDecl") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "StateFrm")) { return state_formulas::mu(parse_Id(node.child(1).child(0)), parse_StateVarAssignmentList(node.child(1).child(1)), parse_StateFrm(node.child(3))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "nu") && (symbol_name(node.child(1)) == "StateVarDecl") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "StateFrm")) { return state_formulas::nu(parse_Id(node.child(1).child(0)), parse_StateVarAssignmentList(node.child(1).child(1)), parse_StateFrm(node.child(3))); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "Id")) { return data::untyped_data_parameter(parse_Id(node.child(0)), parse_DataExprList(node.child(1))); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "delay")) { return make_delay(node.child(1)); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "yaled")) { return make_yaled(node.child(1)); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "(") && (symbol_name(node.child(1)) == "StateFrm") && (symbol_name(node.child(2)) == ")")) { return parse_StateFrm(node.child(1)); }
    throw core::parse_node_unexpected_exception(m_parser, node);
  }

  state_formula parse_FormSpec(const core::parse_node& node) const
  {
    return parse_StateFrm(node.child(1));
  }

  bool callback_StateFrmSpec(const core::parse_node& node, untyped_state_formula_specification& result) const
  {
    if (symbol_name(node) == "SortSpec")
    {
      return callback_DataSpecElement(node, result);
    }
    else if (symbol_name(node) == "ConsSpec")
    {
      return callback_DataSpecElement(node, result);
    }
    else if (symbol_name(node) == "MapSpec")
    {
      return callback_DataSpecElement(node, result);
    }
    else if (symbol_name(node) == "EqnSpec")
    {
      return callback_DataSpecElement(node, result);
    }
    else if (symbol_name(node) == "ActSpec")
    {
      result.action_labels = result.action_labels + parse_ActSpec(node);
      return true;
    }
    else if (symbol_name(node) == "FormSpec")
    {
      result.formula = parse_FormSpec(node);
      return true;
    }
    else if (symbol_name(node) == "StateFrm")
    {
      result.formula = parse_StateFrm(node);
      return true;
    }
    return false;
  }

  untyped_state_formula_specification parse_StateFrmSpec(const core::parse_node& node) const
  {
    untyped_state_formula_specification result;
    traverse(node, [&](const core::parse_node& node) { return callback_StateFrmSpec(node, result); });
    return result;
  }
};

inline
state_formula parse_state_formula(const std::string& text)
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn, core::detail::syntax_error_fn);
  unsigned int start_symbol_index = p.start_symbol_index("StateFrm");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  core::warn_and_or(node);
  state_formula result = state_formula_actions(p).parse_StateFrm(node);
  return result;
}

inline
state_formula_specification parse_state_formula_specification(const std::string& text)
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn, core::detail::syntax_error_fn);
  unsigned int start_symbol_index = p.start_symbol_index("StateFrmSpec");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  core::warn_and_or(node);
  core::warn_left_merge_merge(node);

  untyped_state_formula_specification untyped_statespec = state_formula_actions(p).parse_StateFrmSpec(node);
  state_formula_specification result = untyped_statespec.construct_state_formula_specification();
  return result;
}

} // namespace detail

struct parse_state_formula_options
{
  bool check_monotonicity = true;
  bool translate_regular_formulas = true;
  bool type_check = true;
  bool translate_user_notation = true;
  bool resolve_name_clashes = true;
};

inline
state_formula post_process_state_formula(
  const state_formula& formula,
  parse_state_formula_options options = parse_state_formula_options()
)
{
  state_formula x = formula;
  if (options.translate_regular_formulas)
  {
    mCRL2log(log::debug) << "formula before translating regular formulas: " << x << std::endl;
    x = translate_regular_formulas(x);
    mCRL2log(log::debug) << "formula after translating regular formulas: " << x << std::endl;
  }
  if (options.translate_user_notation)
  {
    x = state_formulas::translate_user_notation(x);
  }
  if (options.check_monotonicity && !is_monotonous(x))
  {
    throw mcrl2::runtime_error("state formula is not monotonic: " + state_formulas::pp(x));
  }
  if (options.resolve_name_clashes && has_name_clashes(x))
  {
    mCRL2log(log::debug) << "formula before resolving name clashes: " << x << std::endl;
    x = state_formulas::resolve_name_clashes(x);
    mCRL2log(log::debug) << "formula after resolving name clashes: " << x << std::endl;
  }
  return x;
}

/// \brief Parses a state formula from an input stream
/// \param text A string.
/// \param lpsspec A linear process specification used as context. The data specification of lpsspec is extended with sorts appearing in the formula.
/// \param options A set of options guiding parsing.
/// \return The parse result.
inline
state_formula parse_state_formula(const std::string& text,
                                  lps::specification& lpsspec,
                                  parse_state_formula_options options = parse_state_formula_options()
                                 )
{
  state_formula x = detail::parse_state_formula(text);
  if (options.type_check)
  {
    x = state_formulas::typecheck_state_formula(x, lpsspec);
  }
  detail::check_actions(x, lpsspec);
  lpsspec.data().add_context_sorts(state_formulas::find_sort_expressions(x));
  return post_process_state_formula(x, options);
}

/// \brief Parses a state formula from an input stream
/// \param in A stream.
/// \param lpsspec A linear process specification used as context. The data specification of lpsspec is extended with sorts appearing in the formula.
/// \param options A set of options guiding parsing.
/// \return The parse result.
inline
state_formula parse_state_formula(std::istream& in,
                                  lps::specification& lpsspec,
                                  parse_state_formula_options options = parse_state_formula_options()
                                 )
{
  std::string text = utilities::read_text(in);
  return parse_state_formula(text, lpsspec, options);
}

/// \brief Parses a state formula specification from a string.
/// \param text A string.
/// \param options A set of options guiding parsing.
/// \return The parse result.
inline
state_formula_specification parse_state_formula_specification(
  const std::string& text,
  parse_state_formula_options options = parse_state_formula_options()
)
{
  state_formula_specification result = detail::parse_state_formula_specification(text);
  if (options.type_check)
  {
    result.formula() = state_formulas::typecheck_state_formula(result.formula(), result.data(), result.action_labels(), data::variable_list());
  }
  result.formula() = post_process_state_formula(result.formula(), options);
  return result;
}

/// \brief Parses a state formula specification from an input stream.
/// \param in An input stream.
/// \param options A set of options guiding parsing.
/// \return The parse result.
inline
state_formula_specification parse_state_formula_specification(
  std::istream& in,
  parse_state_formula_options options = parse_state_formula_options()
)
{
  std::string text = utilities::read_text(in);
  return parse_state_formula_specification(text, options);
}

/// \brief Parses a state formula specification from a string
/// \param text A string
/// \param lpsspec A linear process containing data and action declarations necessary to type check the state formula.
/// \param options A set of options guiding parsing.
/// \return The parse result
inline
state_formula_specification parse_state_formula_specification(const std::string& text,
                                  lps::specification& lpsspec,
                                  parse_state_formula_options options = parse_state_formula_options()
                                 )
{
  state_formula_specification result = detail::parse_state_formula_specification(text);
  data::typecheck_data_specification(result.data());

  data::data_specification dataspec = data::merge_data_specifications(lpsspec.data(), result.data());
  process::action_label_list actspec = process::merge_action_specifications(lpsspec.action_labels(), result.action_labels());
  if (options.type_check)
  {
    result.formula() = state_formulas::typecheck_state_formula(result.formula(), dataspec, actspec, lpsspec.global_variables());
    // TODO: dataspec and actspec must also be typechecked here.
  }
  detail::check_actions(result.formula(), lpsspec);
  result.formula() = post_process_state_formula(result.formula(), options);
  return result;
}

/// \brief Parses a state formula specification from an input stream.
/// \param in An input stream.
/// \param lpsspec A linear process containing data and action declarations necessary to type check the state formula.
/// \param options A set of options guiding parsing.
/// \return The parse result.
inline
state_formula_specification parse_state_formula_specification(std::istream& in,
                                  lps::specification& lpsspec,
                                  parse_state_formula_options options = parse_state_formula_options()
                                 )
{
  return parse_state_formula_specification(utilities::read_text(in), lpsspec, options);
}

} // namespace state_formulas

} // namespace mcrl2

#endif // MCRL2_MODAL_FORMULA_PARSE_H
