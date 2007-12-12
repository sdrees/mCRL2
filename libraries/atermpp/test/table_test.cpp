// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file table_test.cpp
/// \brief Add your file description here.

// very simple test for atermpp::table

#include <iostream>
#include <string>
#include <boost/test/minimal.hpp>

#include <map>
#include "atermpp/atermpp.h"
#include "atermpp/table.h"
#include "atermpp/indexed_set.h"

using namespace std;
using namespace atermpp;

void test_table()
{
  table t(100, 75);
  t.put(make_term("a"), make_term("f(a)"));
  BOOST_CHECK(t.table_keys().size() == 1); 

  {
    table t1 = t;
  }
  table t2 = t;

  aterm a = t.get(make_term("a"));
  BOOST_CHECK(a = make_term("f(a)"));

  std::map<int, table> x;
  x[2] = t;
}

int test_main( int, char*[] )
{
  ATERM_LIBRARY_INIT()
  test_table();
  return 0;
}
