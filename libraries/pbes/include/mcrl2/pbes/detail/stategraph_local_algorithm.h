// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/detail/stategraph_local_algorithm.h
/// \brief add your file description here.

#ifndef MCRL2_PBES_DETAIL_STATEGRAPH_LOCAL_ALGORITHM_H
#define MCRL2_PBES_DETAIL_STATEGRAPH_LOCAL_ALGORITHM_H

#include "mcrl2/data/undefined.h"
#include "mcrl2/pbes/algorithms.h"
#include "mcrl2/pbes/detail/stategraph_algorithm.h"

namespace mcrl2 {

namespace pbes_system {

namespace detail {

struct default_rules_predicate
{
  const local_control_flow_graph& V;

  default_rules_predicate(const local_control_flow_graph& V_)
    : V(V_)
  {}

  bool operator()(const core::identifier_string& X, std::size_t i) const
  {
    return V.has_label(X, i);
  }
};

/// \brief Algorithm class for the local variant of the stategraph algorithm
class stategraph_local_algorithm: public stategraph_algorithm
{
  public:
    typedef stategraph_algorithm super;

  protected:
    // m_control_flow_index[X][j] = k => d_X[j] corresponds to m_control_flow_graphs[k]
    std::map<core::identifier_string, std::map<std::size_t, std::size_t> > m_control_flow_index;

    // m_belongs[k] corresponds with m_control_flow_graphs[k]
    std::vector<std::map<core::identifier_string, std::set<data::variable> > > m_belongs;

    void compute_control_flow_index()
    {
      for (std::size_t k = 0; k < m_connected_components.size(); k++)
      {
        const std::set<std::size_t>& component = m_connected_components[k];
        for (auto i = component.begin(); i != component.end(); ++i)
        {
          const control_flow_graph_vertex& u = m_global_control_flow_graph_vertices[*i];
          m_control_flow_index[u.name()][u.index()] = k;
        }
      }
    }

    void print_control_flow_index() const
    {
      std::ostringstream out;
      for (std::map<core::identifier_string, std::map<std::size_t, std::size_t> >::const_iterator i = m_control_flow_index.begin(); i != m_control_flow_index.end(); ++i)
      {
        const core::identifier_string& X = i->first;
        const std::map<std::size_t, std::size_t>& m = i->second;
        for (std::map<std::size_t, std::size_t>::const_iterator j = m.begin(); j != m.end(); ++j)
        {
          out << "(" << core::pp(X) << ", " << j->first << ") is part of graph " << j->second << std::endl;
        }
      }
      mCRL2log(log::debug, "stategraph") << "=== control flow index ===\n" << out.str() << std::endl;
    }

    // returns an index k such that d_X[j] corresponds to m_control_flow_graphs[k],
    // or data::undefined_index() if no such index was found
    std::size_t control_flow_index(const core::identifier_string& X, std::size_t j) const
    {
      std::map<core::identifier_string, std::map<std::size_t, std::size_t> >::const_iterator k = m_control_flow_index.find(X);
      if (k == m_control_flow_index.end())
      {
        return data::undefined_index();
      }
      const std::map<std::size_t, std::size_t>& m = k->second;
      std::map<std::size_t, std::size_t>::const_iterator i = m.find(j);
      if (i == m.end())
      {
        return data::undefined_index();
      }
      return i->second;
    }

    // prints a belong set
    std::string print_belong_set(const stategraph_equation& eq, const std::set<std::size_t>& belongs) const
    {
      std::set<data::variable> v;
      for (auto i = belongs.begin(); i != belongs.end(); ++i)
      {
        v.insert(eq.parameters()[*i]);
      }
      return core::detail::print_set(v);
    }

    template <typename RulesPredicate>
    std::map<core::identifier_string, std::set<data::variable> > compute_belongs(const local_control_flow_graph& Vk, const std::set<data::data_expression>& values_k, RulesPredicate rules)
    {
      std::map<core::identifier_string, std::set<data::variable> > Bk;
      for (auto p = Vk.vertices.begin(); p != Vk.vertices.end(); ++p)
      {
        auto const& X = p->name();
        auto const& eq_X = *find_equation(m_pbes, X);
        std::set<std::size_t> belongs = data_parameter_indices(X);
        mCRL2log(log::debug1, "stategraph") << "  initial belong set for vertex " << *p << " = " << print_belong_set(eq_X, belongs) << std::endl;

        auto const& predvars = eq_X.predicate_variables();
        for (std::size_t i = 0; i < predvars.size(); i++)
        {
          const predicate_variable& X_i = predvars[i];
          for (std::set<std::size_t>::iterator j = belongs.begin(); j != belongs.end(); )
          {
            std::size_t m = *j;
            if ((X_i.used.find(m) != X_i.used.end() || X_i.changed.find(m) != X_i.changed.end()) && !rules(X, i))
            {
              mCRL2log(log::debug1, "stategraph") << " remove (X, i, m) = (" << X << ", " << i << ", " << m << ") variable=" << eq_X.parameters()[m] << " from belongs " << std::endl;
              belongs.erase(j++);
            }
            else
            {
              ++j;
            }
          }
        }
        mCRL2log(log::debug1, "stategraph") << "  final   belong set for vertex " << *p << " = " << print_belong_set(eq_X, belongs) << std::endl;
        for (std::set<std::size_t>::const_iterator j = belongs.begin(); j != belongs.end(); ++j)
        {
          Bk[X].insert(eq_X.parameters()[*j]);
        }
      }

      return Bk;
    }

    void compute_belongs()
    {
      assert(m_local_control_flow_graphs.size() == m_connected_components_values.size());
      mCRL2log(log::debug, "stategraph") << "=== computing belongs relation ===" << std::endl;
      for (std::size_t k = 0; k < m_local_control_flow_graphs.size(); k++)
      {
        mCRL2log(log::debug, "stategraph") << "--- compute belongs for graph " << k << " ---" << std::endl;
        auto const& Vk = m_local_control_flow_graphs[k];
        auto const& values_k = m_connected_components_values[k];
        std::map<core::identifier_string, std::set<data::variable> > Bk = compute_belongs(Vk, values_k, default_rules_predicate(Vk));
        m_belongs.push_back(Bk);
      }
    }

    void print_belongs() const
    {
      std::ostringstream out;
      out << "\n=== belongs relation ===\n";
      for (std::size_t k = 0; k < m_belongs.size(); k++)
      {
        out << "--- belongs " << k << " ---" << std::endl;
        const std::map<core::identifier_string, std::set<data::variable> >& Bk = m_belongs[k];
        for (std::map<core::identifier_string, std::set<data::variable> >::const_iterator i = Bk.begin(); i != Bk.end(); ++i)
        {
          out << core::pp(i->first) << " -> " << core::detail::print_set(i->second) << std::endl;
        }
      }
      mCRL2log(log::debug, "stategraph") << out.str() << std::endl;
    }

    // Returns true if there is a local control flow graph Gk such that
    // 1) k != k0
    // 2) (X, d) \in Bk
    // 3) Gk has a vertex u = X(e) such that d \in marking(u)
    bool is_marked_in_other_graph(const core::identifier_string& X, const data::variable& d, std::size_t k0)
    {
      std::size_t K = m_local_control_flow_graphs.size();
      for (std::size_t k = 0; k < K; k++)
      {
        if (k == k0)
        {
          continue;
        }
        if (!utilities::detail::contains(m_belongs[k][X], d))
        {
          continue;
        }
        const local_control_flow_graph& Gk = m_local_control_flow_graphs[k];
        auto const& inst = Gk.index(X);
        for (auto i = inst.begin(); i != inst.end(); ++i)
        {
          auto const& u = **i;
          if (utilities::detail::contains(u.marking(), d))
          {
            mCRL2log(log::debug, "stategraph") << "is_marked_in_other_graph(" << core::pp(X) << ", " << data::pp(d) << ") = true" << std::endl;
            mCRL2log(log::debug, "stategraph") << "witness: u = " << u.name() << " in graph " << k << std::endl;
            return true;
          }
        }
      }
      return false;
    }

    std::string print_vertex(const local_control_flow_graph_vertex& u) const
    {
      std::ostringstream out;
      out << u.name()
          << " marking = " << core::detail::print_set(u.marking())
          << " marking indices = " << print_set(u.marking_variable_indices(m_pbes));
      return out.str();
    }

    std::set<data::variable> significant_variables(const local_control_flow_graph_vertex& u) const
    {
      const core::identifier_string& X = u.name();
      const pbes_equation& eq_X = *find_equation(m_pbes, X);
      pbes_expression phi = eq_X.formula();
mCRL2log(log::debug, "stategraph") << "  compute significant variables of vertex u = " << u << std::endl;
pbes_expression phi0 = phi;
      if (u.index() != data::undefined_index())
      {
        data::variable d = u.variable();
        data::data_expression e = u.value();
        data::mutable_map_substitution<> sigma;
        sigma[d] = e;
        pbes_system::simplifying_rewriter<pbes_expression, data::rewriter> pbesr(m_datar);
        phi = pbesr(phi, sigma);
      }
mCRL2log(log::debug, "stategraph") << "  rhs           = " << phi0 << std::endl;
mCRL2log(log::debug, "stategraph") << "  simplify(rhs) = " << phi << std::endl;
      // return pbes_system::algorithms::significant_variables(phi);
      std::set<data::variable> result = pbes_system::algorithms::significant_variables(phi);
mCRL2log(log::debug, "stategraph") << "  significant variables: " << core::detail::print_set(result) << std::endl;
      return result;
    }

    // returns the index of variable d in the vector d_Y
    std::size_t find_parameter_index(const std::vector<data::variable>& d_Y, const data::variable& d) const
    {
      auto i = std::find(d_Y.begin(), d_Y.end(), d);
      if (i == d_Y.end())
      {
        throw mcrl2::runtime_error("find_parameter_index: variable not found!");
      }
      return i - d_Y.begin();
    }

    bool belongs_contains(const std::map<core::identifier_string, std::set<data::variable> >& B,
                          const core::identifier_string& X,
                          const data::variable& d
                         ) const
    {
      auto i = B.find(X);
      if (i == B.end())
      {
        return false;
      }
      return utilities::detail::contains(i->second, d);
    }

    // returns the intersection of V with { d | (X, d) in B }
    std::set<data::variable> belongs_intersection(const std::set<data::variable>& V,
                                                  const std::map<core::identifier_string, std::set<data::variable> >& B,
                                                  const core::identifier_string& X
                                                 )
    {
      auto i = B.find(X);
      if (i == B.end())
      {
        return std::set<data::variable>();
      }
      return data::detail::set_intersection(V, i->second);
    }

    template <typename Substitution>
    data::data_expression rewr(const data::data_expression& x, Substitution sigma)
    {
      return m_datar(x, sigma);
    }

    // updates u.marking
    // returns true if u.marking has changed
    bool update_marking_rule1(const local_control_flow_graph& V,
                              const std::map<core::identifier_string, std::set<data::variable> >& B,
                              const local_control_flow_graph_vertex& u
                             )
    {
      std::size_t size = u.marking().size();
      auto const& X = u.name();
      auto const& eq_X = *find_equation(m_pbes, X);
      auto const& outgoing_edges = u.outgoing_edges();
      for (auto ei = outgoing_edges.begin(); ei != outgoing_edges.end(); ++ei)
      {
        auto const& v = *ei->first;
        auto const& labels = ei->second;
        for (auto ii = labels.begin(); ii != labels.end(); ++ii)
        {
          // consider edge (u, i, v)
          std::size_t i = *ii;
          auto const& Y = v.name();
          auto const& eq_Y = *find_equation(m_pbes, Y);
          const predicate_variable& Ye = eq_X.predicate_variables()[i];
          auto const& e = Ye.X.parameters();
          for (auto di = v.marking().begin(); di != v.marking().end(); ++di)
          {
            auto const& d = *di;
            std::size_t l = find_parameter_index(eq_Y.parameters(), d);
            data::mutable_map_substitution<> sigma;
            sigma[u.variable()] = u.value();
            auto W = FV(rewr(nth_element(e, l), sigma));
            u.set_marking(data::detail::set_union(u.marking(), belongs_intersection(W, B, X)));
          }
        }
      }
      return u.marking().size() != size;
    }

    // updates u.marking
    // returns true if u.marking has changed
    bool update_marking_rule2(std::size_t i,
                              const local_control_flow_graph& V,
                              const std::map<core::identifier_string, std::set<data::variable> >& B,
                              const local_control_flow_graph& V1,
                              const std::map<core::identifier_string, std::set<data::variable> >& B1,
                              const local_control_flow_graph_vertex& u,
                              const local_control_flow_graph_vertex& v
                             )
    {
      std::size_t size = u.marking().size();
      auto const& X = u.name();
      auto const& eq_X = *find_equation(m_pbes, X);
      auto const& Y = v.name();
      auto const& eq_Y = *find_equation(m_pbes, Y);
      const predicate_variable& Ye = eq_X.predicate_variables()[i];
      auto const& e = Ye.X.parameters();
      for (auto di = v.marking().begin(); di != v.marking().end(); ++di)
      {
        auto const& d = *di;
        if (belongs_contains(B, Y, d))
        {
          continue;
        }
        std::size_t l = find_parameter_index(eq_Y.parameters(), d);
        data::mutable_map_substitution<> sigma;
        sigma[u.variable()] = u.value();
        auto W = FV(rewr(nth_element(e, l), sigma));
        u.set_marking(data::detail::set_union(u.marking(), belongs_intersection(W, B, X)));
      }
      return u.marking().size() != size;
    }

    void compute_control_flow_marking()
    {
      mCRL2log(log::debug, "stategraph") << "=== computing control flow marking ===" << std::endl;
      using data::detail::set_difference;
      using data::detail::set_intersection;
      using data::detail::set_union;

      std::size_t J = m_local_control_flow_graphs.size();
      for (std::size_t j = 0; j < J; j++)
      {
        auto const& Vj = m_local_control_flow_graphs[j];
        auto const& Bj = m_belongs[j];
        for (auto ui = Vj.vertices.begin(); ui != Vj.vertices.end(); ++ui)
        {
          auto& u = *ui;
          auto const& X = u.name();
          u.set_marking(belongs_intersection(significant_variables(u), Bj, X));
        }
        mCRL2log(log::debug, "stategraph") << "--- initial control flow marking " << j << "\n" << Vj.print_marking();
      }

      bool stable = false;
      while (!stable)
      {
        bool stableint = false;
        while (!stableint)
        {
          stableint = true;
          for (std::size_t j = 0; j < J; j++)
          {
            auto const& Vj = m_local_control_flow_graphs[j];
            auto const& Bj = m_belongs[j];
            std::set<const local_control_flow_graph_vertex*> todo;
            for (auto q = Vj.vertices.begin(); q != Vj.vertices.end(); ++q)
            {
              todo.insert(&(*q));
            }
            while (!todo.empty())
            {
              auto ti = todo.begin();
              todo.erase(ti);
              const local_control_flow_graph_vertex& u = **ti;
              auto m = u.marking();
              bool changed = update_marking_rule1(Vj, Bj, u);
              if (changed)
              {
                auto const& outgoing_edges = u.outgoing_edges();
                for (auto ei = outgoing_edges.begin(); ei != outgoing_edges.end(); ++ei)
                {
                  auto const& v = *ei->first;
                  todo.insert(&v);
                }
                stableint = false;
              }
            }
          }
        }

        bool stableext = false;
        while (!stableext)
        {
          stableext = true;
          for (std::size_t j = 0; j < J; j++)
          {
            auto const& Vj = m_local_control_flow_graphs[j];
            auto const& Bj = m_belongs[j];
            for (auto vj = Vj.vertices.begin(); vj != Vj.vertices.end(); ++vj)
            {
              auto const& u = *vj;
              auto const& X = u.name();
              auto const& eq_X = *find_equation(m_pbes, X);
              auto const& predvars = eq_X.predicate_variables();
              for (std::size_t i = 0; i < predvars.size(); i++)
              {
                auto const& Ye = predvars[i];
                auto const& Y = Ye.X.name();
                for (std::size_t k = j + 1; k < J; k++)
                {
                  auto const& Vk = m_local_control_flow_graphs[k];
                  auto const& Bk = m_belongs[k];
                  for (auto vk = Vk.vertices.begin(); vk != Vk.vertices.end(); ++vk)
                  {
                    auto const& v = *vk;
                    if (v.name() != Y)
                    {
                      continue;
                    }
                    for (auto di = v.marking().begin(); di != v.marking().end(); ++di)
                    {
                      auto const& d = *di;
                      if (!belongs_contains(Bk, Y, d))
                      {
                        auto m = u.marking();
                        bool changed = update_marking_rule2(i, Vj, Bj, Vk, Bk, u, v);
                        if (changed)
                        {
                          stableint = false;
                          stableext = false;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
        stable = stableint;
      }
    }

    void compute_control_flow_marking_old()
    {
      mCRL2log(log::debug, "stategraph") << "=== computing control flow marking ===" << std::endl;
      using data::detail::set_difference;
      using data::detail::set_intersection;
      using data::detail::set_union;

      std::size_t K = m_local_control_flow_graphs.size();

      // initial marking
      for (std::size_t k = 0; k < K; k++)
      {
        local_control_flow_graph& Gk = m_local_control_flow_graphs[k];
        for (auto i = Gk.vertices.begin(); i != Gk.vertices.end(); ++i)
        {
          const local_control_flow_graph_vertex& u = *i;
          const core::identifier_string& X = u.name();
          const pbes_equation& eq_X = *find_equation(m_pbes, X);
          pbes_expression phi = eq_X.formula();
          if (u.index() != data::undefined_index())
          {
            data::variable d = u.variable();
            data::data_expression e = u.value();
            data::mutable_map_substitution<> sigma;
            sigma[d] = e;
            pbes_system::simplifying_rewriter<pbes_expression, data::rewriter> pbesr(m_datar);
            phi = pbesr(phi, sigma);
          }
          std::set<data::variable> sig = pbes_system::algorithms::significant_variables(phi);
          u.set_marking(set_intersection(sig, m_belongs[k][X]));
        }
        mCRL2log(log::debug, "stategraph") << "--- initial control flow marking " << k << "\n" << Gk.print_marking();
      }

      bool stable = false;
      while (!stable)
      {
        stable = true;
        for (std::size_t k = 0; k < K; k++)
        {
          // initialize todo list
          local_control_flow_graph& Gk = m_local_control_flow_graphs[k];
          std::set<const local_control_flow_graph_vertex*> todo;
          for (auto gi = Gk.vertices.begin(); gi != Gk.vertices.end(); ++gi)
          {
            const local_control_flow_graph_vertex& v = *gi;
            todo.insert(&v);
          }

          while (!todo.empty())
          {
            auto ti = todo.begin();
            todo.erase(ti);

            // v = X(e)
            const local_control_flow_graph_vertex& v = **ti;
            std::set<std::size_t> v_index = v.marking_variable_indices(m_pbes);
            mCRL2log(log::debug, "stategraph") << "selected todo element v = " << print_vertex(v) << std::endl;
            auto const& incoming_edges = v.incoming_edges();
            for (auto ei = incoming_edges.begin(); ei != incoming_edges.end(); ++ei)
            {
              for (auto q = ei->second.begin(); q != ei->second.end(); ++q)
              {
                std::size_t i = *q;

                // u = Y(f)
                // edge = (u, v) with label i
                const local_control_flow_graph_vertex& u = *ei->first;
                std::set<std::size_t> u_index = u.marking_variable_indices(m_pbes);
                const stategraph_equation& eq_Y = *find_equation(m_pbes, u.name());

                // Y_i = X(g)
                const predicate_variable& Y_i = eq_Y.predicate_variables()[i];
                const core::identifier_string& Y = u.name();
                mCRL2log(log::debug, "stategraph") << "  vertex u = " << print_vertex(u) << std::endl;
                for (std::set<std::size_t>::const_iterator ji = v_index.begin(); ji != v_index.end(); ++ji)
                {
                  std::size_t j = *ji;
                  data::data_expression g_j = nth_element(Y_i.X.parameters(), j);
                  std::set<data::variable> M = set_intersection(set_difference(FV(g_j), u.marking()), m_belongs[k][Y]);
                  if (!M.empty())
                  {
                    mCRL2log(log::debug, "stategraph") << "update marking u with M = " << core::detail::print_set(M) << std::endl;
                    u.set_marking(set_union(u.marking(), M));
                    todo.insert(&u);
                    stable = false;
                  }
                }
              }
            }
          }

          for (auto gi = Gk.vertices.begin(); gi != Gk.vertices.end(); ++gi)
          {
            // u = X(e)
            const local_control_flow_graph_vertex& u = *gi;
            const core::identifier_string& X = u.name();
            const stategraph_equation& eq_X = *find_equation(m_pbes, X);
            for (std::size_t i = 0; i < eq_X.predicate_variables().size(); i++)
            {
              const predicate_variable& X_i = eq_X.predicate_variables()[i];
              const core::identifier_string& Y = X_i.X.name();
              const data::data_expression_list& f = X_i.X.parameters();
              const stategraph_equation& eq_Y = *find_equation(m_pbes, Y);
              const std::vector<data::variable>& d_Y = eq_Y.parameters();
              for (std::size_t l = 0; l < d_Y.size(); ++l)
              {
                std::map<core::identifier_string, std::set<data::variable> >& Bk = m_belongs[k];
                if (!utilities::detail::contains(Bk[Y], d_Y[l]))
                {
                  std::set<data::variable> M = set_difference(set_intersection(m_belongs[k][X], FV(nth_element(f, l))), u.marking());
                  if (M.empty())
                  {
                    continue;
                  }
                  if (is_marked_in_other_graph(Y, d_Y[l], k))
                  {
                    u.set_marking(set_union(u.marking(), M));
                    mCRL2log(log::debug, "stategraph") << "update marking u with M = " << core::detail::print_set(M) << std::endl;
                    stable = false;
                  }
                }
              }
            }
          }
        }
      }
    }

    void print_control_flow_marking() const
    {
      mCRL2log(log::debug, "stategraph") << "\n=== control flow marking ===" << std::endl;
      std::size_t K = m_local_control_flow_graphs.size();
      for (std::size_t k = 0; k < K; k++)
      {
        const local_control_flow_graph& Gk = m_local_control_flow_graphs[k];
        mCRL2log(log::debug, "stategraph") <<  "--- control flow marking for graph " << k << "\n" << Gk.print_marking() << std::endl;
      }
    }

  public:
    stategraph_local_algorithm(const pbes& p, data::rewriter::strategy rewrite_strategy = data::jitty,
                               bool use_alternative_lcfp_criterion = false,
                               bool use_alternative_gcfp_relation = false,
                               bool use_alternative_gcfp_consistency = false
                              )
      : stategraph_algorithm(p, rewrite_strategy, use_alternative_lcfp_criterion, use_alternative_gcfp_relation, use_alternative_gcfp_consistency)
    { }

    /// \brief Computes the control flow graph
    void run()
    {
      super::run();
      compute_control_flow_index();
      print_control_flow_index();
      compute_belongs();
      print_belongs();
      compute_control_flow_marking();
      print_control_flow_marking();
    }
};

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_STATEGRAPH_LOCAL_ALGORITHM_H
