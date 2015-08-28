// Author(s): Muck van Weerdenburg; completely rewritten by Jan Friso Groote
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file liblts_lts.cpp

#include <string>
#include <cstring>
#include <sstream>
#include "mcrl2/core/nil.h"
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/data/detail/io.h"
#include "mcrl2/lts/lts_lts.h"
#include "mcrl2/lps/multi_action.h"
#include "mcrl2/data/data_expression.h"

namespace mcrl2
{
namespace lts
{
namespace detail
{

using namespace atermpp;

static atermpp::function_symbol transition_header()
{
  static atermpp::function_symbol tr("transition ",3);
  return tr;
}

static atermpp::function_symbol num_of_states_labels_and_initial_state()
{
  static atermpp::function_symbol nslais("num_of_states_labels_and_initial_state",3);
  return nslais;
}

static atermpp::function_symbol lts_header()
{
  static atermpp::function_symbol lts("labelled_transition_system ",6);
  return lts;
}

static atermpp::function_symbol meta_data_header()
{
  static atermpp::function_symbol mdh("meta_data_header ",4);
  return mdh;
}

/// Below we introduce aterm representations for a transition,
/// which can subsequently be stored as an aterm.
class aterm_transition: public atermpp::aterm_appl
{
  public:
    aterm_transition(const size_t source, const size_t label, const size_t target)
      : aterm_appl(transition_header(),aterm_int(source),aterm_int(label),aterm_int(target))
    {}

    size_t source() const
    {
      return (atermpp::down_cast<aterm_int>((*this)[0]).value());
    }

    size_t label() const
    {
      return (atermpp::down_cast<aterm_int>((*this)[1]).value());
    }

    size_t target() const
    {
      return (atermpp::down_cast<aterm_int>((*this)[2]).value());
    }
};

typedef term_list<aterm_transition> aterm_transition_list;
typedef term_list<term_balanced_tree<data::data_expression> > state_labels_t; // The state labels have the shape STATE(a1,...,an).
typedef term_list<process::action_list> action_labels_t;         // An action label is a lists of actions.
typedef term_list<data::data_expression> probabilistic_labels_t; // This contains a list of probabilities.
typedef term_list<data::function_symbol> boolean_list_t;         // A list with constants true or false, indicating
                                                                 // whether a state is probabilistic.

class aterm_labelled_transition_system: public atermpp::aterm_appl
{
  public:
    aterm_labelled_transition_system(const aterm& a)
      : aterm_appl(a)
    {}

    aterm_labelled_transition_system(
               const lts_lts_t& ts,
               const aterm_transition_list& transitions,
               const state_labels_t& state_label_list,
               const action_labels_t& action_label_list,
               const probabilistic_labels_t& probabilistic_label_list,
               const boolean_list_t& probabilistic_state_indicator_list)
      : aterm_appl(lts_header(),
                   aterm_appl(meta_data_header(),
                              ts.has_data() ? data::detail::data_specification_to_aterm_data_spec(ts.data()) : core::nil(),
                              ts.has_process_parameters() ? static_cast<aterm>(ts.process_parameters()) : core::nil(),
                              ts.has_action_labels() ? static_cast<aterm>(ts.action_labels()) : core::nil(),
                              aterm_appl(num_of_states_labels_and_initial_state(),
                                         aterm_int(ts.num_states()),
                                         aterm_int(ts.num_action_labels()),
                                         aterm_int(ts.initial_state()))),
                   transitions,
                   state_label_list,
                   action_label_list,
                   probabilistic_label_list,
                   probabilistic_state_indicator_list
                  )
    {}

    bool has_data() const
    {
      return meta_data()[0]!=core::nil();
    } 

    const data::data_specification data() const
    {
      assert(meta_data()[0]!=core::nil());
      return data::data_specification(down_cast<aterm_appl>(meta_data()[0]));
    }
    
    bool has_process_parameters() const
    {
      return meta_data()[1]!=core::nil();
    } 

    const data::variable_list process_parameters() const
    {
      assert(meta_data()[1]!=core::nil());
      return down_cast<data::variable_list>(meta_data()[1]);
    }
    
    bool has_action_labels() const
    {
      return meta_data()[2]!=core::nil();
    } 

    const process::action_label_list action_labels() const
    {
      assert(meta_data()[2]!=core::nil());
      return down_cast<process::action_label_list>(meta_data()[2]);
    }

    size_t num_states() const
    {
      const aterm_appl t=down_cast<aterm_appl>(meta_data()[3]);
      assert(t.function()==num_of_states_labels_and_initial_state());
      return down_cast<aterm_int>(t[0]).value();
    }

    size_t num_action_labels() const
    {
      const aterm_appl t=down_cast<aterm_appl>(meta_data()[3]);
      assert(t.function()==num_of_states_labels_and_initial_state());
      return down_cast<aterm_int>(t[1]).value();
    }

    size_t initial_state() const
    {
      const aterm_appl t=down_cast<aterm_appl>(meta_data()[3]);
      assert(t.function()==num_of_states_labels_and_initial_state());
      return down_cast<aterm_int>(t[2]).value();
    }
    
    aterm_transition_list transitions() const
    {
      return down_cast<aterm_transition_list>((*this)[1]);
    }
  
    state_labels_t get_state_labels() const
    {
      return down_cast<state_labels_t>((*this)[2]);
    }
  
    action_labels_t get_action_labels() const
    {
      return down_cast<action_labels_t>((*this)[3]);
    }
  
    probabilistic_labels_t get_probabilistic_labels() const
    {
      return down_cast<probabilistic_labels_t>((*this)[4]);
    }
  
    boolean_list_t get_probabilistic_state_indicators() const
    {
      return down_cast<boolean_list_t>((*this)[5]);
    }
  
  protected:
    
    const aterm_appl& meta_data() const
    {
      assert(atermpp::down_cast<aterm_appl>((*this)[0]).function()==meta_data_header());
      return atermpp::down_cast<aterm_appl>((*this)[0]);
    }
};

static void read_from_lts(lts_lts_t& l, const std::string& filename)
{
  aterm input;
  if (filename=="")
  {
    input=read_term_from_binary_stream(std::cin);
  }
  else 
  {
    std::ifstream stream;
    stream.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
    try
    {  
      stream.open(filename, std::ifstream::in | std::ifstream::binary);
    }
    catch (std::ifstream::failure)
    {
      if (filename=="")
      {
        throw mcrl2::runtime_error("Fail to open standard input to read an lts.");
      }
      else 
      {
        throw mcrl2::runtime_error("Fail to open file " + filename + " to read an lts.");
      }
    }

    try
    {
      input=atermpp::read_term_from_binary_stream(stream);
      stream.close();
    }
    catch (std::ifstream::failure)
    {
      if (filename=="")
      {
        throw mcrl2::runtime_error("Fail to correctly read an lts from standard input.");
      }
      else
      {
        throw mcrl2::runtime_error("Fail to correctly read an lts from the file " + filename + ".");
      }
    }
    
  }
  input=data::detail::add_index(input);
  
  if (!input.type_is_appl() || down_cast<aterm_appl>(input).function()!=lts_header())
  {
    throw runtime_error("The input file " + filename + " is not in proper .lts format.");
  }
  
  const aterm meta_data=atermpp::down_cast<aterm_appl>(input)[0];
  
  if (!meta_data.type_is_appl() || down_cast<aterm_appl>(meta_data).function()!=meta_data_header())
  {
    throw runtime_error("The input file " + filename + " is not in proper .lts format. There is a problem with the datatypes, process parameters and action declarations.");
  }
  
  const aterm_labelled_transition_system input_lts(input);
  if (input_lts.has_data())
  {
    l.set_data(input_lts.data());
  }

  if (input_lts.has_process_parameters())
  {
    l.set_process_parameters(input_lts.process_parameters());
  }

  if (input_lts.has_action_labels())
  {
    l.set_action_labels(input_lts.action_labels());
  }
  
  const aterm_transition_list& input_transitions=input_lts.transitions();
  for(const aterm_transition& t: input_transitions)
  {
    l.add_transition(transition(t.source(), t.label(), t.target()));
  }
  
  if (input_lts.get_state_labels().size()==0)
  {
    l.set_num_states(input_lts.num_states());
  }
  else
  {
    assert(input_lts.num_states()==input_lts.get_state_labels().size());
    for (const term_balanced_tree<data::data_expression>& state_label:input_lts.get_state_labels())
    {
      l.add_state(state_label);
    }
  }

  if (input_lts.get_action_labels().size()==0)
  {
    l.set_num_action_labels(input_lts.num_action_labels());
  }
  else
  {
    assert(input_lts.num_action_labels()==input_lts.get_action_labels().size());
    const process::action_list tau = process::action_list();
    for (const process::action_list& action_label:input_lts.get_action_labels())
    {
      l.add_action(lps::multi_action(action_label), action_label == tau); 
    }
  }

  l.set_initial_state(input_lts.initial_state());

  for(const data::data_expression d:input_lts.get_probabilistic_labels()) 
  {
    l.add_probabilistic_label(d); 
  }

  size_t i=0;
  for(const data::function_symbol& f:input_lts.get_probabilistic_state_indicators())
  {
    assert(f==data::sort_bool::true_() || f==data::sort_bool::false_());
    if (f==data::sort_bool::true_())
    {
      l.set_is_probabilistic(i,true);
    }
    ++i;
  }

}

static void write_to_lts(const lts_lts_t& l, const std::string& filename)
{
  aterm_transition_list transitions;
  
  for(std::vector<transition>::const_reverse_iterator i=l.get_transitions().rbegin();
                i!=l.get_transitions().rend(); ++i)
  {
    // The transitions are stored in reverse order.
    transitions.push_front(aterm_transition(i->from(),i->label(),i->to()));
  }

  state_labels_t state_label_list;
  if (l.has_state_info())
  { for(size_t i=l.num_state_labels(); i>0;)
    {
      --i;
      state_label_list.push_front(l.state_label(i));
    }
  }

  action_labels_t action_label_list;
  if (l.has_action_info())
  { 
    for(size_t i=l.num_action_labels(); i>0;)
    {
      --i;
      action_label_list.push_front(l.action_label(i).actions());
    }
  }

  probabilistic_labels_t probabilistic_label_list;
  for(size_t i=l.num_probabilistic_labels(); i>0;)
  {
    --i;
    probabilistic_label_list.push_front(l.probabilistic_label(i));
  }

  boolean_list_t probabilistic_indicator_list;
  for(size_t i=l.num_states(); i>0;)
  {
    --i;
    probabilistic_indicator_list.push_front((l.is_probabilistic(i)?data::sort_bool::true_():data::sort_bool::false_()));
  }

  const aterm_labelled_transition_system t0(l,
                                            transitions,
                                            state_label_list,
                                            action_label_list,
                                            probabilistic_label_list,
                                            probabilistic_indicator_list);
  const aterm t1 = data::detail::remove_index(t0);
  
  if (filename=="")
  {
    atermpp::write_term_to_binary_stream(t1, std::cout);
  }
  else 
  {
    std::ofstream stream;
    stream.exceptions ( std::ofstream::failbit | std::ofstream::badbit );
    try
    {
      stream.open(filename, std::ofstream::out | std::ofstream::binary);
    }
    catch (std::ofstream::failure )
    {
      throw mcrl2::runtime_error("Fail to open file " + filename + " for writing.");
    }
    try
    { 
      atermpp::write_term_to_binary_stream(t1, stream);
      stream.close();
    }
    catch (std::ofstream::failure)
    {
      throw mcrl2::runtime_error("Fail to write lts correctly to the file " + filename + ".");
    }
  }
}

} // namespace detail

void lts_lts_t::save(const std::string& filename) const
{
  mCRL2log(log::verbose) << "Starting to save file " << filename << "\n";
  detail::write_to_lts(*this,filename);
}

void lts_lts_t::load(const std::string& filename)
{
  mCRL2log(log::verbose) << "Starting to load file " << filename << "\n";
  detail::read_from_lts(*this,filename);
}

}
}
