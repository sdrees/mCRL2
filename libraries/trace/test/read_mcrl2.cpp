// Author(s): Unknown
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file read_mcrl2.cpp

#define BOOST_TEST_MODULE read_mcrl2

#include <boost/test/included/unit_test_framework.hpp>
#include <exception>
#include <sstream>
#include "mcrl2/process/action_parse.h"
#include "mcrl2/trace/trace.h"
#include "mcrl2/core/print.h"

using namespace std;
using namespace mcrl2;
using namespace mcrl2::trace;


void test_next_action(Trace& t, const char* s)
{
  mcrl2::lps::multi_action a = t.currentAction();
  t.increasePosition();
  BOOST_CHECK((a != mcrl2::lps::multi_action()));
  if (a != mcrl2::lps::multi_action())
  {
    string action(pp(a));
    BOOST_CHECK(action == s);
    if (action != s)
    {
      std::cout << "--- error detected ---\n";
      std::cout << "result         : " << action << std::endl;
      std::cout << "expected result: " << s << std::endl;
    }
  }
}

BOOST_AUTO_TEST_CASE(test_main)
{
  using namespace mcrl2::data;
  using namespace mcrl2::lps;
  using namespace mcrl2::core;
  mcrl2::data::data_specification data_spec;
  process::action_label_list act_decls;
  act_decls.push_front(process::action_label(identifier_string("a"),sort_expression_list()));
  sort_expression_list s;
  s.push_front(sort_bool::bool_());
  s.push_front(sort_pos::pos());
  act_decls.push_front(process::action_label("b",s));
  act_decls.push_front(process::action_label(identifier_string("c"),sort_expression_list()));

  Trace t(data_spec, act_decls);
  t.addAction(parse_multi_action("a",act_decls,data_spec));
  t.addAction(parse_multi_action("b(1,true)",act_decls,data_spec));
  t.addAction(parse_multi_action("c",act_decls,data_spec));

  stringstream trace_data;
  t.save(trace_data);

  try
  {
    t.load(trace_data,tfMcrl2);
  }
  catch (const mcrl2::runtime_error& e)
  {
    BOOST_ERROR(e.what());
  }

  BOOST_CHECK(t.number_of_actions() == 3);

  test_next_action(t,"a");
  test_next_action(t,"b(1, true)");
  test_next_action(t,"c");
}
