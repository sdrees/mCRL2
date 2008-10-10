#ifndef MCRL2_DATA_SET_H
#define MCRL2_DATA_SET_H

#include "mcrl2/data/basic_sort.h"
#include "mcrl2/data/function_sort.h"
#include "mcrl2/data/function_symbol.h"
#include "mcrl2/data/application.h"
#include "mcrl2/data/data_equation.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/data/bool_.h"

namespace mcrl2 {

  namespace data {

    namespace sort_set {

      // Sort expression Set(s)
      inline
      container_sort set(const sort_expression& s)
      {
        static container_sort set("set", s);
        return set;
      }

      // Recogniser for sort expression Set(s)
      inline
      bool is_set(const sort_expression& e)
      {
        if (e.is_container_sort())
        {
          return static_cast<const container_sort&>(e).name() == "set";
        }
        return false;
      }

      // Function symbol @set
      inline
      function_symbol set_comprehension(const sort_expression& s)
      {
        static function_symbol set_comprehension("@set", function_sort(s, set(s)));
        return set_comprehension;
      }

      // Recogniser for @set
      inline
      bool is_set_comprehension_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == set_comprehension();
        }
        return false;
      }

      // Application of @set
      inline
      application set_comprehension(const sort_expression& s, const data_expression& arg0)
      {
        assert(arg0.sort() == s);
        
        return application(set_comprehension(s),arg0);
      }

      // Recogniser for application of @set
      inline
      bool is_set_comprehension_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_set_comprehension_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol {}
      inline
      function_symbol emptyset(const sort_expression& s)
      {
        static function_symbol emptyset("{}", set(s));
        return emptyset;
      }

      // Recogniser for {}
      inline
      bool is_emptyset_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == emptyset();
        }
        return false;
      }

      // Application of {}
      inline
      application emptyset(const sort_expression& s, const data_expression& arg0)
      {
        assert(arg0.sort() == s);
        
        return application(emptyset(s),arg0);
      }

      // Recogniser for application of {}
      inline
      bool is_emptyset_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_emptyset_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol in
      inline
      function_symbol in(const sort_expression& s)
      {
        static function_symbol in("in", function_sort(s, set(s), bool));
        return in;
      }

      // Recogniser for in
      inline
      bool is_in_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == in();
        }
        return false;
      }

      // Application of in
      inline
      application in(const sort_expression& s, const data_expression& arg0)
      {
        assert(arg0.sort() == s);
        
        return application(in(s),arg0);
      }

      // Recogniser for application of in
      inline
      bool is_in_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_in_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol <=
      inline
      function_symbol subset_or_equal(const sort_expression& s)
      {
        static function_symbol subset_or_equal("<=", function_sort(set(s), set(s), bool));
        return subset_or_equal;
      }

      // Recogniser for <=
      inline
      bool is_subset_or_equal_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == subset_or_equal();
        }
        return false;
      }

      // Application of <=
      inline
      application subset_or_equal(const sort_expression& s, const data_expression& arg0)
      {
        assert(arg0.sort() == s);
        
        return application(subset_or_equal(s),arg0);
      }

      // Recogniser for application of <=
      inline
      bool is_subset_or_equal_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_subset_or_equal_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol <
      inline
      function_symbol subset(const sort_expression& s)
      {
        static function_symbol subset("<", function_sort(set(s), set(s), bool));
        return subset;
      }

      // Recogniser for <
      inline
      bool is_subset_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == subset();
        }
        return false;
      }

      // Application of <
      inline
      application subset(const sort_expression& s, const data_expression& arg0)
      {
        assert(arg0.sort() == s);
        
        return application(subset(s),arg0);
      }

      // Recogniser for application of <
      inline
      bool is_subset_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_subset_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol +
      inline
      function_symbol union_(const sort_expression& s)
      {
        static function_symbol union_("+", function_sort(set(s), set(s), set(s)));
        return union_;
      }

      // Recogniser for +
      inline
      bool is_union__function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == union_();
        }
        return false;
      }

      // Application of +
      inline
      application union_(const sort_expression& s, const data_expression& arg0)
      {
        assert(arg0.sort() == s);
        
        return application(union_(s),arg0);
      }

      // Recogniser for application of +
      inline
      bool is_union__application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_union__function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol -
      inline
      function_symbol difference(const sort_expression& s)
      {
        static function_symbol difference("-", function_sort(set(s), set(s), set(s)));
        return difference;
      }

      // Recogniser for -
      inline
      bool is_difference_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == difference();
        }
        return false;
      }

      // Application of -
      inline
      application difference(const sort_expression& s, const data_expression& arg0)
      {
        assert(arg0.sort() == s);
        
        return application(difference(s),arg0);
      }

      // Recogniser for application of -
      inline
      bool is_difference_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_difference_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol *
      inline
      function_symbol intersection(const sort_expression& s)
      {
        static function_symbol intersection("*", function_sort(set(s), set(s), set(s)));
        return intersection;
      }

      // Recogniser for *
      inline
      bool is_intersection_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == intersection();
        }
        return false;
      }

      // Application of *
      inline
      application intersection(const sort_expression& s, const data_expression& arg0)
      {
        assert(arg0.sort() == s);
        
        return application(intersection(s),arg0);
      }

      // Recogniser for application of *
      inline
      bool is_intersection_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_intersection_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol -
      inline
      function_symbol complement(const sort_expression& s)
      {
        static function_symbol complement("-", function_sort(set(s), set(s)));
        return complement;
      }

      // Recogniser for -
      inline
      bool is_complement_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == complement();
        }
        return false;
      }

      // Application of -
      inline
      application complement(const sort_expression& s, const data_expression& arg0)
      {
        assert(arg0.sort() == s);
        
        return application(complement(s),arg0);
      }

      // Recogniser for application of -
      inline
      bool is_complement_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_complement_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function for projecting out right
      inline
      data_expression right(const data_expression& e)
      {
        assert(is_in_application(e) || is_subset_or_equal_application(e) || is_subset_application(e) || is_union__application(e) || is_difference_application(e) || is_intersection_application(e));
        
        if (is_in_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_subset_or_equal_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_subset_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_union__application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_difference_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_intersection_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        // This should never be reached, otherwise something is severely wrong.
        assert(false); 
      }

      // Function for projecting out left
      inline
      data_expression left(const data_expression& e)
      {
        assert(is_in_application(e) || is_subset_or_equal_application(e) || is_subset_application(e) || is_union__application(e) || is_difference_application(e) || is_intersection_application(e));
        
        if (is_in_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_subset_or_equal_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_subset_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_union__application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_difference_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_intersection_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        // This should never be reached, otherwise something is severely wrong.
        assert(false); 
      }

      // Function for projecting out arg
      inline
      data_expression arg(const data_expression& e)
      {
        assert(is_set_comprehension_application(e) || is_complement_application(e));
        
        if (is_set_comprehension_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_complement_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        // This should never be reached, otherwise something is severely wrong.
        assert(false); 
      }

      // Give all system defined equations for Set
      inline
      data_equation_list set_generate_equations_code(const sort_expression& s)
      {
        data_equation_list result;
        result.push_back(data_equation(make_vector(variable(g, function_sort(s, bool_())), variable(f, function_sort(s, bool_()))), true_(), application(equal_to, application(set_comprehension, variable(f, function_sort(s, bool_()))), application(set_comprehension, variable(g, function_sort(s, bool_())))), application(equal_to, variable(f, function_sort(s, bool_())), variable(g, function_sort(s, bool_())))));
        result.push_back(data_equation(variable_list(), true_(), emptyset, application(set_comprehension, lambda(make_vector(variable(x, s)),false_()))));
        result.push_back(data_equation(make_vector(variable(d, s), variable(f, function_sort(s, bool_()))), true_(), application(in, variable(d, s), application(set_comprehension, variable(f, function_sort(s, bool_())))), application(variable(f, function_sort(s, bool_())), variable(d, s))));
        result.push_back(data_equation(make_vector(variable(g, function_sort(s, bool_())), variable(f, function_sort(s, bool_()))), true_(), application(subset_or_equal, application(set_comprehension, variable(f, function_sort(s, bool_()))), application(set_comprehension, variable(g, function_sort(s, bool_())))), forall(make_vector(variable(x, s)),application(implies, application(variable(f, function_sort(s, bool_())), variable(x, s)), application(variable(g, function_sort(s, bool_())), variable(x, s))))));
        result.push_back(data_equation(make_vector(variable(s, set(s)), variable(t, set(s))), true_(), application(subset, variable(s, set(s)), variable(t, set(s))), application(and_, application(subset_or_equal, variable(s, set(s)), variable(t, set(s))), application(not_equal_to, variable(s, set(s)), variable(t, set(s))))));
        result.push_back(data_equation(make_vector(variable(g, function_sort(s, bool_())), variable(f, function_sort(s, bool_()))), true_(), application(union_, application(set_comprehension, variable(f, function_sort(s, bool_()))), application(set_comprehension, variable(g, function_sort(s, bool_())))), application(set_comprehension, lambda(make_vector(variable(x, s)),application(or_, application(variable(f, function_sort(s, bool_())), variable(x, s)), application(variable(g, function_sort(s, bool_())), variable(x, s)))))));
        result.push_back(data_equation(make_vector(variable(s, set(s)), variable(t, set(s)), variable(g, function_sort(s, bool_())), variable(f, function_sort(s, bool_()))), true_(), application(difference, application(set_comprehension, variable(f, function_sort(s, bool_()))), application(set_comprehension, variable(g, function_sort(s, bool_())))), application(intersection, variable(s, set(s)), application(difference, variable(t, set(s))))));
        result.push_back(data_equation(make_vector(variable(g, function_sort(s, bool_())), variable(f, function_sort(s, bool_()))), true_(), application(intersection, application(set_comprehension, variable(f, function_sort(s, bool_()))), application(set_comprehension, variable(g, function_sort(s, bool_())))), application(set_comprehension, lambda(make_vector(variable(x, s)),application(and_, application(variable(f, function_sort(s, bool_())), variable(x, s)), application(variable(g, function_sort(s, bool_())), variable(x, s)))))));
        result.push_back(data_equation(make_vector(variable(f, function_sort(s, bool_()))), true_(), application(difference, application(set_comprehension, variable(f, function_sort(s, bool_())))), application(set_comprehension, lambda(make_vector(variable(x, s)),application(not_, application(variable(f, function_sort(s, bool_())), variable(x, s)))))));

        return result;
      }

    } // namespace set
  } // namespace data
} // namespace mcrl2

#endif // MCRL2_DATA_SET_H
