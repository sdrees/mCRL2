// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/process/detail/sort_expression_builder.inc.h
/// \brief The content of this file is included in other header
/// files, to prevent duplication.

void operator()(process_equation& x)
{
  x = process_equation(static_cast<Derived&>(*this)(x.identifier()),
                       static_cast<Derived&>(*this)(x.formal_parameters()),
                       static_cast<Derived&>(*this)(x.expression())
                      );
}

void operator()(process_specification& x)
{
  static_cast<Derived&>(*this)(x.global_variables());
  static_cast<Derived&>(*this)(x.equations());
  x.init() = static_cast<Derived&>(*this)(x.init());
}     

lps::action_label operator()(const lps::action_label& x)
{
  return lps::action_label(x.name(), static_cast<Derived&>(*this)(x.sorts()));
}     

process_identifier operator()(const process_identifier& x)
{
  return process_identifier(x.name(), static_cast<Derived&>(*this)(x.sorts()));
}

process_expression operator()(const process_instance& x)
{
  static_cast<Derived&>(*this).enter(x);
  process_expression result = process_instance(static_cast<Derived&>(*this)(x.identifier()), static_cast<Derived&>(*this)(x.actual_parameters()));
  static_cast<Derived&>(*this).leave(x);
  return result;
}

process_expression operator()(const process_instance_assignment& x)
{
  static_cast<Derived&>(*this).enter(x);
  process_expression result = process_instance_assignment(static_cast<Derived&>(*this)(x.identifier()), static_cast<Derived&>(*this)(x.assignments()));
  static_cast<Derived&>(*this).leave(x);
  return result;
}

process_expression operator()(const sum& x)
{
  static_cast<Derived&>(*this).enter(x);
  process_expression result = sum(static_cast<Derived&>(*this)(x.bound_variables()), static_cast<Derived&>(*this)(x.operand()));
  static_cast<Derived&>(*this).leave(x);
  return result;
}

process_expression operator()(const at& x)
{
  static_cast<Derived&>(*this).enter(x);
  process_expression result = at(static_cast<Derived&>(*this)(x.operand()), static_cast<Derived&>(*this)(x.time_stamp()));
  static_cast<Derived&>(*this).leave(x);
  return result;
}

process_expression operator()(const if_then& x)
{
  static_cast<Derived&>(*this).enter(x);
  process_expression result = if_then(static_cast<Derived&>(*this)(x.condition()), static_cast<Derived&>(*this)(x.then_case()));
  static_cast<Derived&>(*this).leave(x);
  return result;
}

process_expression operator()(const if_then_else& x)
{
  static_cast<Derived&>(*this).enter(x);
  process_expression result = if_then_else(static_cast<Derived&>(*this)(x.condition()), static_cast<Derived&>(*this)(x.then_case()), static_cast<Derived&>(*this)(x.else_case()));
  static_cast<Derived&>(*this).leave(x);
  return result;
}
