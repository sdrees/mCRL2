// Author(s): Maurice Laveaux
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "mcrl2/lps/io.h"
#include "mcrl2/utilities/logger.h"
#include "mcrl2/utilities/input_output_output_tool.h"

#include "dependency_cleave.h"
#include "lpscleave_utility.h"

#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace mcrl2
{

using log::log_level_t;
using lps::stochastic_specification;
using mcrl2::utilities::tools::input_output_output_tool;

class lpscleave_tool : public input_output_output_tool
{
  typedef input_output_output_tool super;

public:
  lpscleave_tool() : super(
      "lpscleave",
      "Maurice Laveaux",
      "Cleaves LPSs",
      "Decomposes the data parameters of the linear process specification (LPS) "
      "in INFILE and write the result of the left component to OUTFILE1 and the right"
      "component to OUTFILE2. If INFILE is not present, stdin is used.")
  {}

  bool run() override
  {
    stochastic_specification spec;

    if (input_filename().empty())
    {
      load_lps(spec, std::cin);
    }
    else
    {
      load_lps(spec, input_filename());
    }

    if (m_parameters.empty())
    {
      // Print the parameters and exit
      print_names("Parameters", spec.process().process_parameters());
      mCRL2log(log::info) << "Number of summands: " << spec.process().summand_count() << "\n";
    }
    else
    {
      // Here, we should decide on a good cleaving.

      // For now, the parameters are given by the user.
      auto parameters = project_parameters(spec.process().process_parameters(), m_parameters);

      // Cleave the process, requires the indices to be sorted.
      m_indices.sort();
      m_indices.unique();

      // Compute the independent summands for this process.
      print_elements("Independent summands", compute_indices(spec, parameters));

      // Save the resulting left-cleave.
      {
        stochastic_specification left_spec = dependency_cleave(spec, parameters, m_indices, false);
        std::ofstream file(output_filename1(), std::ios::binary);
        left_spec.save(file, true);
      }

      // Take the complement of the indices.
      m_indices = get_other_indices(spec.process(), m_indices);

      // Take the complement of the parameters
      parameters = get_other_parameters(spec.process(), parameters);

      // Save the resulting right-cleave.
      {
        stochastic_specification right_spec = dependency_cleave(spec, parameters, m_indices, true);
        std::ofstream file(output_filename2(), std::ios::binary);
        right_spec.save(file, true);
      }
    }

    return true;
  }

protected:
  void add_options(utilities::interface_description& desc) override
  {
    super::add_options(desc);

    desc.add_option("parameters", utilities::make_mandatory_argument("PARAMS"), "A comma separated list of PARAMS that are used for the left process of the cleave.", 'p');
    desc.add_option("summands", utilities::make_mandatory_argument("INDICES"), "A comma separated list of INDICES of summands where the left process generates the action.", 's');
  }

  void parse_options(const utilities::command_line_parser& parser) override
  {
    super::parse_options(parser);

    if (parser.options.count("parameters"))
    {
      m_parameters = split_actions(parser.option_argument("parameters"));
    }

    if (parser.options.count("summands"))
    {
      std::list<std::string> indices = split_actions(parser.option_argument("summands"));
      for (auto& string : indices)
      {
        m_indices.emplace_back(std::stoul(string));
      }
    }
  }

private:
  std::list<std::string> m_parameters;
  std::list<std::size_t> m_indices;
};

} // namespace mcrl2

int main(int argc, char** argv)
{
  return mcrl2::lpscleave_tool().execute(argc, argv);
}
