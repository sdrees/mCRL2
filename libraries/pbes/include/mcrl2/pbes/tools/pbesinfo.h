// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/tools/pbesinfo.h
/// \brief add your file description here.

#ifndef MCRL2_PBES_TOOLS_PBESINFO_H
#define MCRL2_PBES_TOOLS_PBESINFO_H

#include "mcrl2/pbes/detail/pbes_property_map.h"
#include "mcrl2/pbes/io.h"

namespace mcrl2 {

namespace pbes_system {

void pbesinfo(const std::string& input_filename,
              const std::string& input_file_message,
              const utilities::file_format& file_format,
              bool opt_full
             )
{
  pbes p;
  load_pbes(p, input_filename, file_format);

  detail::pbes_property_map info(p);

  // Show file from which PBES was read
  std::cout << input_file_message << "\n\n";

  // Show number of equations
  std::cout << "Number of equations:     " << p.equations().size() << std::endl;

  // Show number of mu's with the predicate variables from the mu's
  std::cout << "Number of mu's:          " << info["mu_equation_count"] << std::endl;

  // Show number of nu's with the predicate variables from the nu's
  std::cout << "Number of nu's:          " << info["nu_equation_count"] << std::endl;

  // Show number of nu's with the predicate variables from the nu's
  std::cout << "Block nesting depth:     " << info["block_nesting_depth"] << std::endl;

  // Show if PBES is closed and well formed
  std::cout << "The PBES is closed:      " << std::flush;
  std::cout << (p.is_closed() ? "yes" : "no ") << std::endl;
  std::cout << "The PBES is well formed: " << std::flush;
  std::cout << (p.is_well_typed() ? "yes" : "no ") << std::endl;

  // Show binding variables with their signature
  if (opt_full)
  {
    std::cout << "Predicate variables:\n";
    for (std::vector<pbes_equation>::const_iterator i = p.equations().begin(); i != p.equations().end(); ++i)
    {
      std::cout << core::pp(i->symbol()) << "." << pp(i->variable()) << std::endl;
    }
  }
}

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_TOOLS_PBESINFO_H
