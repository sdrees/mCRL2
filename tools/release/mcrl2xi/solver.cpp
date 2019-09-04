// Author(s): Rimco Boudewijns
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <QCoreApplication>

#include "solver.h"
#include "parsing.h"

#include "mcrl2/data/enumerator_with_iterator.h"

Solver::Solver(QThread *atermThread, mcrl2::data::rewrite_strategy strategy):
  m_strategy(strategy)
{
  moveToThread(atermThread);
  m_parsed = false;
}

void Solver::solve(QString specification, QString dataExpression)
{
  using namespace mcrl2::data;
  m_abort = false;
  if (m_specification != specification || !m_parsed)
  {
    m_parsed = false;
    m_specification = specification;
    try
    {
      mcrl2xi_qt::parseMcrl2Specification(m_specification.toStdString(), m_data_spec, m_global_vars);
      m_parsed = true;
    }
    catch (const mcrl2::runtime_error& e)
    {
      m_parseError = QString::fromStdString(e.what());
    }
  }

  if (m_parsed)
  {
    try
    {
      std::string stdDataExpression = dataExpression.toStdString();

      mCRL2log(info) << "Solving: \"" << stdDataExpression << "\"" << std::endl;

      std::size_t dotpos = stdDataExpression.find('.');
      if (dotpos  == std::string::npos)
      {
        throw mcrl2::runtime_error("Expected input of the shape 'x1:Type1,...,xn:Typen.b' where b is a boolean expression.");
      }

      std::set <variable > m_vars=m_global_vars;
      parse_variables(std::string(stdDataExpression.substr(0, dotpos)
                                  ) + ";",std::inserter(m_vars,m_vars.begin()),m_data_spec);

      data_expression term =
          parse_data_expression(
            stdDataExpression.substr(dotpos+1, stdDataExpression.length()-1),
            m_vars,
            m_data_spec
            );

      if (term.sort()!=sort_bool::bool_())
      {
        throw mcrl2::runtime_error("Expression is not of sort Bool.");
      }

      std::set<sort_expression> all_sorts=find_sort_expressions(term);
      find_sort_expressions(m_vars, std::inserter(all_sorts, all_sorts.end()));
      m_data_spec.add_context_sorts(all_sorts);

      rewriter rewr(m_data_spec, m_strategy);

      term=rewr(term);

      typedef enumerator_algorithm_with_iterator<rewriter> enumerator_type;
      typedef enumerator_list_element_with_substitution<> enumerator_element;

      // Stop when more than 10000 internal variables are required.
      mcrl2::data::enumerator_identifier_generator id_generator;
      enumerator_type enumerator(rewr, m_data_spec, rewr, id_generator, 10000, true);

      mutable_indexed_substitution<> sigma;
      mcrl2::data::enumerator_queue<enumerator_element> enumerator_deque(enumerator_element(variable_list(m_vars.begin(),m_vars.end()), term));
      for (enumerator_type::iterator i = enumerator.begin(sigma, enumerator_deque); i != enumerator.end() && !m_abort; ++i)
      {
        mCRL2log(verbose) << "Solution found" << std::endl;

        QString s('[');

        mutable_indexed_substitution<> sigma_i;
        i->add_assignments(m_vars,sigma_i,rewr);
        for (std::set< variable >::const_iterator v=m_vars.begin(); v!=m_vars.end() ; ++v)
        {
          if( v != m_vars.begin() )
          {
            s.append(", ");
          }
          s.append(pp(*v).c_str());
          s.append(" := ");
          s.append(pp(sigma_i(*v)).c_str());
        }
        s.append("] evaluates to ");
        s.append(pp(rewr(term,sigma_i)).c_str());

        emit solvedPart(s);

        QCoreApplication::processEvents(); // To process the signals
        if (m_abort)
          break;
      }
      mCRL2log(info) << (m_abort ? "Abort by user." : "Done solving.") << std::endl;
    }
    catch (const mcrl2::runtime_error& e)
    {
      QString err = QString::fromStdString(e.what());
      emit(exprError(err));
    }
  }
  else
  {
    emit(parseError(m_parseError));
  }
  emit(finished());
}

void Solver::abort()
{
  m_abort = true;
}
