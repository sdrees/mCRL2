#include <algorithm>
#include <fstream>
#include <map>
#include <stack>
#include <set>
#include <memory>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/ref.hpp>
#include <boost/filesystem/operations.hpp>

#include <xml2pp/text_reader.h>

#include "project_manager.h"
#include "settings_manager.tcc"

namespace squadt {

  namespace bf = boost::filesystem;

  const std::string project_manager::maintain_old_name("");

  /**
   * @param l a path to the root of the project store
   *
   * \pre l should be a path to a directory
   * 
   * If the directory does not exist then it is created and an initial project
   * description file is written to it. If the directory exists but there is no
   * project description file in it, then such a file is created and all files
   * in the directory are imported into the project.
   **/
  project_manager::project_manager(const boost::filesystem::path& l, bool b) : store(l), count(0) {
    using namespace boost;

    assert(!l.empty());

    filesystem::path project_file = l / filesystem::path(settings_manager::project_definition_base_name);

    if (filesystem::exists(l)) {
      assert(filesystem::is_directory(l));

      if (!filesystem::exists(project_file)) {
        import_directory(l);

        /* Create initial project description file */
        write();
      }
      else {
        if (b) {
          filesystem::remove(project_file);
        }
        else {
          try {
            read();
          }
          catch (...) {
            b = true;
          }
        }

        if (b) {
          /* Project description file is probably broken */
          import_directory(l);

          /* Create initial project description file */
          write();
        }
      }
    }
    else {
      filesystem::create_directories(l);

      /* Create initial project description file */
      write();
    }

    /* Compute reverse dependencies */
    for (processor_list::const_iterator i = processors.begin(); i != processors.end(); ++i) {
      for (processor::input_object_iterator j = (*i)->get_input_iterator(); j.valid(); ++j) {
        processor::sptr g((*j)->generator.lock());

        if (g.get() != 0) {
          reverse_depends.insert(std::make_pair(g.get(), (*i).get()));
        }
      }
    }
  }

  /**
   * @param p a reference to the processor to add
   **/
  void project_manager::add(processor::ptr const& p) {
    boost::mutex::scoped_lock l(list_lock);

    if (processors.end() == std::find(processors.begin(), processors.end(), p)) {
      processors.push_back(p);

      /* Register reverse dependencies */
      for (processor::input_object_iterator i = p->get_input_iterator(); i.valid(); ++i) {
        processor::sptr g((*i)->generator.lock());

        if (g.get() != 0) {
          reverse_depends.insert(std::make_pair(g.get(), p.get()));
        }
      }

      write();
    }
  }

  /**
   * @param l the directory that is to be imported
   **/
  void project_manager::import_directory(const boost::filesystem::path& l) {
    assert(bf::exists(l) && bf::is_directory(l));

    bf::directory_iterator end;
    bf::directory_iterator i(l);

    while (i != end) {
      if (is_directory(*i) && !symbolic_link_exists(*i)) {
        /* Recursively import */
        import_directory(*i);
      }
      else {
        if ((*i).leaf() != settings_manager::project_definition_base_name) {
          import_file(*i);
        }
      }

      ++i;
    }
  }

  void project_manager::write() const {
    std::ofstream project_stream(settings_manager::path_concatenate(store,
             settings_manager::project_definition_base_name).c_str(), std::ios::out | std::ios::trunc);
 
    write(project_stream);
 
    project_stream.close();
  }

  /**
   * @param[out] s the stream the output is written to
   **/
  void project_manager::write(std::ostream& s) const {
    /* Write header */
    s << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      << "<squadt-project xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
      << " xsi:noNamespaceSchemaLocation=\"project.xsd\" version=\"1.0\" count=\""
      << count << "\">\n";

    if (!description.empty()) {
      s << "<description>" << description << "</description>\n";
    }
 
    BOOST_FOREACH(processor_list::value_type p, processors) {
      p->write(s);
    }
 
    s << "</squadt-project>\n";
  }

  /**
   * \pre directory/<|settings_manager::project_definition_base_name|> must exist
   **/
  void project_manager::read() {
    bf::path p(settings_manager::path_concatenate(store, settings_manager::project_definition_base_name));

    assert(bf::exists(p) && !bf::is_directory(p));

    try {
      xml2pp::text_reader reader(p);

      reader.set_schema(bf::path(global_settings_manager->path_to_schemas(
                                    settings_manager::append_schema_suffix(
                                            settings_manager::project_definition_base_name)).c_str()));

      read(reader);
    }
    catch (...) {
      throw;
    }
  }

  /**
   * @param[in] l a path to a project file
   **/
  project_manager::ptr project_manager::read(const std::string& l) {
    bf::path p(settings_manager::path_concatenate(l, settings_manager::project_definition_base_name));

    if (!bf::exists(p)) {
      throw (exception::exception(exception::failed_loading_object, "SQuADT project", p.native_file_string()));
    }

    project_manager::ptr m(new project_manager());

    try {
      xml2pp::text_reader reader(p);

      reader.set_schema(bf::path(global_settings_manager->path_to_schemas(
                                      settings_manager::append_schema_suffix(
                                              settings_manager::project_definition_base_name)).c_str()));
     
      m->read(reader);

      m->store = l;
    }
    catch (...) {
      throw;
    }

    return (m);
  }

  /**
   * @param[in] r an xml2pp text reader that has been constructed with a project file
   **/
  void project_manager::read(xml2pp::text_reader& r) {

    if (!r.is_element("squadt-project")) {
      throw (exception::exception(exception::failed_loading_object, "SQuADT project", "expected squadt-project element"));
    }

    r.get_attribute(&count, "count");

    /* Advance beyond project element */
    r.next_element();

    if (r.is_element("description") && !r.is_empty_element()) {
      r.next_element();

      r.get_value(&description);

      r.next_element(1);
    }

    processor::id_conversion_map c;

    /* Read processors */
    while (r.is_element("processor")) {
      processor::sptr p = processor::read(*this, c, r);

      p->check_status(true);

      processors.push_back(p);
    }

    sort_processors();
  }

  /**
   * Sort processors such that for all i < j:
   *
   *   f(processors[i]) < f(processors[j]), or
   *
   *   f(processors[i]) = f(processors[j]) implies
   *
   *     not exists l < k : processor[l] -> processors[j] and processor[l].number_of_inputs() = 0 and
   *                        processor[k] -> processors[i] and processor[k].number_of_inputs() = 0
   *
   *  where:
   *
   *    - processor[k] -> processor[i] is a dependency of processor i on k
   *
   *    - function f is defined as:
   *      - f(p) = 0, if p.number_of_inputs() == 0
   *      - f(p) = 1 + f(max i : f(p.inputs[i].generator))
   **/
  void project_manager::sort_processors() {
    unsigned int number = 0; /* The number of */

    std::map < processor*, unsigned short > weights;

    /* Compute weights */
    processor_list::const_iterator j = processors.begin(); /* Lower bound */

    for (processor_list::const_iterator i = j; i != processors.end(); ++i) {
      if ((*i)->number_of_inputs() == 0) {
        weights[(*i).get()] = ++number;
      }
    }

    while (j != processors.end()) {
      for (processor_list::const_iterator i = j; i != processors.end(); ++i) {
        if (weights.find((*i).get()) != weights.end()) {
          if (i == j) {
            ++j;
          }
        }
        else {
          processor::input_object_iterator k              = (*i)->get_input_iterator();
          unsigned int                     maximum_weight = 0;

          while (k.valid()) {
            processor::sptr target((*k)->generator);

            if (target.get() != 0) {
              if (weights.find(target.get()) == weights.end()) {
                break;
              }
              else {
                unsigned int current_weight = weights[target.get()];
             
                if (maximum_weight < current_weight) {
                  maximum_weight = current_weight;
                }
              }
            }

            ++k;
          }

          if (!k.valid()) {
            weights[(*i).get()] = 1 + std::max(maximum_weight, number);
          }
        }
      }
    }

    /* Do the actual sorting */
    std::stable_sort(processors.begin(), processors.end(), boost::bind(std::less< unsigned short >(), 
                        boost::bind(&std::map < processor*, unsigned short >::operator[], weights,
                                boost::bind(&processor::ptr::get, _1)),
                        boost::bind(&std::map < processor*, unsigned short >::operator[], weights,
                                boost::bind(&processor::ptr::get, _2))));
  }

  /**
   * \param[in] p the processor that selects the targets
   **/
  void project_manager::update_status(processor* p) {
    if (0 < reverse_depends.count(p)) {

      std::stack < processor* > p_stack;

      p_stack.push(p);

      while (0 < p_stack.size()) {
        std::pair < dependency_map::iterator, dependency_map::iterator > range = reverse_depends.equal_range(p_stack.top());

        p_stack.pop();

        BOOST_FOREACH(dependency_map::value_type i, range) {
          i.second->check_status(false);

          p_stack.push(i.second);
        }
      }
    }
  }

  std::auto_ptr < project_manager::conflict_list > project_manager::get_conflict_list(processor::sptr p) const {
    std::set < std::string > locations; // Names of files produced by p

    for (processor::output_object_iterator j = p->get_output_iterator(); j.valid(); ++j) {
      locations.insert((*j)->location);
    }

    boost::mutex::scoped_lock l(list_lock);

    std::auto_ptr < conflict_list > conflicts(new conflict_list);

    /* Check all processors for conflicts */
    for (processor_list::const_iterator i = processors.begin(); i != processors.end(); ++i) {
      if (*i != p) {
        for (processor::output_object_iterator j = (*i)->get_output_iterator(); j.valid(); ++j) {
          if (locations.find((*j)->location) != locations.end()) {
            /* Conflict */
            conflicts->push_back(j.pointer());
          }
        }
      }
    }

    return (conflicts);
  }

  /**
   * \param[in] p the processor that selects the targets
   **/
  void project_manager::update_single(processor::sptr p) {

    /* Recursively update inputs */
    for (processor::input_object_iterator j = p->get_input_iterator(); j.valid(); ++j) {
      processor::sptr dependency((*j)->generator.lock());

      if (dependency.get() != 0) {
        update_single(dependency);
      }
    }

    if (p->check_status(true)) {
      p->update(boost::bind(&project_manager::update_single, this, p));
    }
  }

  /**
   * @param[in] h a function that is called that is called just before a processor is updated
   **/
  void project_manager::update(boost::function < void (processor*) > h) {
    bool update_active = false;

    if (!update_active) {
      update_active = true;

      BOOST_FOREACH(processor_list::value_type i, processors) {
        if (i->get_tool()) {
          h(i.get());

          update_single(i);
        }
      }

      update_active = false;
    }
  }

  /**
   * @param s path that identifies the file that is to be imported
   * @param d new name for the file in the project
   *
   * Note that:
   *  - when d is empty, the original filename will be maintained
   *  - when the file is already in the project store it is not copied
   **/
  processor::ptr project_manager::import_file(const boost::filesystem::path& s, const std::string& d) {
    using namespace boost::filesystem;

    assert(exists(s) && !is_directory(s) && native(d));

    path           destination_path = store / path(d.empty() ? s.leaf() : d);
    processor::ptr p                = processor::create(*this);

    if (s != destination_path && !exists(destination_path)) {
      copy_file(s, destination_path);
    }

    /* Add the file to the project */
    storage_format f = storage_format_unknown;

    /* TODO more intelligent file format check */
    if (!extension(s).empty()) {
      f = extension(s);

      f.erase(f.begin());
    }

    p->append_output(f, destination_path.leaf(), processor::object_descriptor::original);

    processors.push_back(p);

    return (p);
  }

/**
   * @param[in] p pointer to the processor that is to be removed
   * @param[in] b whether or not to remove the associated files
   *
   * \attention also removes processors with inputs that are not longer available
   * \pre input should be sorted as can be obtained by doing sort_processors()
   **/
  void project_manager::remove(processor* p, bool b) {
    std::set < processor* > obsolete;

    obsolete.insert(p);

    if (0 < reverse_depends.count(p)) {

      std::stack < processor* > p_stack;

      p_stack.push(p);

      while (0 < p_stack.size()) {
        std::pair < dependency_map::iterator, dependency_map::iterator > range = reverse_depends.equal_range(p_stack.top());

        p_stack.pop();

        BOOST_FOREACH(dependency_map::value_type i, range) {
          obsolete.insert(i.second);

          p_stack.push(i.second);
        }
      }
    }

    boost::mutex::scoped_lock l(list_lock);

    /* Update reverse dependencies, remove files and processors */
    BOOST_FOREACH(processor* i, obsolete) {
      for (processor::input_object_iterator j = i->get_input_iterator(); j.valid(); ++j) {
        processor::sptr g((*j)->generator.lock());

        if (g.get() != 0) {
          std::pair < dependency_map::iterator, dependency_map::iterator > range(reverse_depends.equal_range(g.get()));

          while (range.first != range.second) {
            if ((*range.first).second == i) {
              dependency_map::iterator temporary = range.first++;

              reverse_depends.erase(temporary);
            }
            else {
              ++range.first;
            }
          }
        }
      }

      if (b) {
        i->flush_outputs();
      }
    }

    processor_list::iterator j = processors.begin();

    /** Actually remove the processor */
    while (j != processors.end()) {
      if (obsolete.find((*j).get()) != obsolete.end()) {
        j = processors.erase(j);
      }
      else {
        ++j;
      }
    }

    write();
  }

  void project_manager::clean_store(processor* p, bool b) {
    namespace bf = boost::filesystem;

    std::set < std::string > objects;

    for (processor_list::iterator i = processors.begin(); i != processors.end(); ++i) {
      for (processor::output_object_iterator j = (*i)->get_output_iterator(); j.valid(); ++j) {
        objects.insert(bf::path((*j)->location).leaf());
      }
    }

    for (bf::directory_iterator i(store); i != bf::directory_iterator(); ++i) {
      if (objects.find((*i).leaf()) == objects.end()) {
        if (bf::exists((*i).leaf()) && !bf::is_directory((*i).leaf()) && !bf::symbolic_link_exists((*i).leaf())) {
          bf::remove((*i).leaf());
        }
      }
    }
  }
}

