// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file processor.h
/// \brief Add your file description here.

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/md5.hpp>

#include "tipi/utility/indirect_iterator.hpp"
#include "task_monitor.hpp"
#include "tool.hpp"

namespace squadt {

  using iterator_wrapper::constant_indirect_iterator;

  class project_manager;

  class processor_impl;

  /**
   * \brief A processor represents a tool configuration.
   *
   * The processor is a container to distinguish configurations that might or
   * might not actually be the same. The most notable part of such a
   * configuration are the inputs and outputs. The inputs are important since
   * the controller must ensure that input exists before running a tool with
   * this configuration.  The outputs are important because they can be the
   * input of other processors.
   *
   **/
  class processor : public utility::visitable, private boost::noncopyable {
    friend class processor_impl;
    friend class project_manager;
    friend class tool_manager;
    friend class executor;

    template < typename R, typename S >
    friend class utility::visitor;
 
    public:

      /** \brief Convenience type for hiding shared pointer implementation */
      typedef boost::shared_ptr < processor >                               ptr;

      /** \brief Convenience type for hiding shared pointer implementation */
      typedef boost::shared_ptr < processor >                               sptr;

      /** \brief Convenience type for hiding shared pointer implementation */
      typedef boost::weak_ptr < processor >                                 wptr;

      /** \brief Convenient type alias */
      typedef tipi::configuration::parameter_identifier                      parameter_identifier;

      /** \brief Type to hold information about output objects */
      struct object_descriptor {

        /** \brief Convenience type for hiding shared pointer implementation */
        typedef boost::shared_ptr < object_descriptor >  sptr;
       
        /** \brief Convenience type for hiding shared pointer implementation */
        typedef boost::weak_ptr < object_descriptor >    wptr;

        /** \brief Type that is used to keep the status of the output of a processor */
        enum t_status {
          original,                 /* unique not reproducible */
          reproducible_nonexistent, /* can be reproduced but does not exist */
          reproducible_out_of_date, /* can be reproduced, exists and is out of date */
          reproducible_up_to_date,  /* can be reproduced, exists, and is up to date */
          generation_in_progress    /* outputs are being generated */
        };

        processor::wptr                          generator;      ///< The process responsible for generating this object
        build_system::storage_format             mime_type;      ///< The used storage format
        std::string                              location;       ///< The location of the object
        tipi::configuration::parameter_identifier identifier;     ///< The identifier of the associated output object in a configuration
        boost::md5::digest_type                  checksum;       ///< The digest for the completed object
        std::time_t                              timestamp;      ///< The last time the file was modified just before the last checksum was computed
        t_status                                 status;         ///< The status of this object

        /** \brief Construction with a mime type */
        object_descriptor(tipi::mime_type const&);

        /** \brief Whether or not the generator points to an existing object */
        bool generator_exists();

        /** \brief Verifies whether or not the object is up to date */
        bool is_up_to_date();

        /** \brief Verifies whether or not the object is present in the store and updates status accordingly */
        bool present_in_store(project_manager const&);

        /** \brief Verifies whether or not the object is physically available and not changed */
        bool self_check(project_manager const&, long int const& = 0);

        /** \brief Synchronises timestamp and checksum with files on disk */
        void synchronise(project_manager const&);
      };

      class monitor;

      /** \brief Convenience type for hiding the implementation of a list with input information */
      typedef std::vector < object_descriptor::sptr >                       input_list;

      /** \brief Convenience type for hiding the implementation of a list with output information */
      typedef std::vector < object_descriptor::sptr >                       output_list;

      /** \brief Type for iterating the input objects */
      typedef constant_indirect_iterator < input_list, object_descriptor >  input_object_iterator;

      /** \brief Type for iterating the output objects */
      typedef constant_indirect_iterator < output_list, object_descriptor > output_object_iterator;

      /** \brief Helper type for read() members */
      typedef std::map < unsigned long, object_descriptor::sptr >           id_conversion_map;

    private:

      /** \brief Private implementation */
      boost::shared_ptr < processor_impl > impl;

    private:

      /** \brief Basic constructor */
      processor();

    public:

      /** \brief Factory method for creating instances of this object */
      static processor::sptr create(boost::weak_ptr < project_manager > const&);
 
      /** \brief Factory method for creating instances of this object */
      static processor::sptr create(boost::weak_ptr < project_manager > const&, tool::sptr);
 
      /** \brief Check the inputs with respect to the outputs and adjust status accordingly */
      bool check_status(bool);

      /** \brief Execute an edit command on one of the outputs */
      void edit(execution::command*);

      /** \brief Sets the status of the outputs to out-of-date if the processor is inactive */
      bool demote_status();

      /** \brief Start tool configuration */
      void configure(const tool::input_combination*, const boost::filesystem::path&, std::string const& = "");
 
      /** \brief Start tool configuration */
      void configure(std::string const& = "");

      /** \brief Start tool reconfiguration */
      void reconfigure(std::string const& = "");
 
      /** \brief Start processing: generate outputs from inputs */
      void run(bool b = false);

      /** \brief Start running and afterward execute a function */
      void run(boost::function < void () > h, bool b = false);

      /** \brief Start processing if not all outputs are up to date */
      void update(bool b = false);
 
      /** \brief Start updating and afterward execute a function */
      void update(boost::function < void () > h, bool b = false);

      /** \brief Get the object for the tool associated with this processor */
      void set_tool(tool::sptr const&);

      /** \brief Get the object for the tool associated with this processor */
      const tool::sptr get_tool();

      /** \brief Get the input combination if one is already selected */
      void set_input_combination(tool::input_combination*);

      /** \brief Whether or not an input combination has been set */
      bool has_input_combination();

      /** \brief Get the input combination if one is already selected */
      tool::input_combination const* get_input_combination() const;

      /** \brief Get the object for the tool associated with this processor */
      boost::shared_ptr < monitor > get_monitor();

      /** \brief Whether or not a tool is running for this object */
      bool is_active() const;

      /** \brief Get input objects */
      input_object_iterator get_input_iterator() const;
 
      /** \brief Get output objects */
      output_object_iterator get_output_iterator() const;
 
      /** \brief Add an input object */
      void append_input(object_descriptor::sptr const&);

      /** \brief Find an object descriptor for a given pointer to an object (by id) */
      const object_descriptor::sptr find_input(parameter_identifier const&) const;
 
      /** \brief Find an object descriptor for a given pointer to an object */
      const object_descriptor::sptr find_input(object_descriptor*) const;
 
      /** \brief Find an object descriptor for a given pointer to an object (by id) */
      const object_descriptor::sptr find_output(parameter_identifier const&) const;
 
      /** \brief Find an object descriptor for a given pointer to an object */
      const object_descriptor::sptr find_output(object_descriptor*) const;
 
      /** \brief Find an object descriptor for a given name and rename if it exists */
      void rename_output(std::string const&, std::string const&);
 
      /** \brief Find an object descriptor for a given name and rename if it exists */
      void rename_input(std::string const&, std::string const&);
 
      /** \brief Add an output object */
      void append_output(const build_system::storage_format&, parameter_identifier const&, const std::string&,
                object_descriptor::t_status const& = object_descriptor::reproducible_nonexistent);
 
      /** \brief Add an output object */
      void replace_output(object_descriptor::sptr, tipi::object const&,
                object_descriptor::t_status const& = object_descriptor::reproducible_up_to_date);

      /** \brief The number of input objects of this processor */
      const size_t number_of_inputs() const;
 
      /** \brief The number output objects of this processor */
      const size_t number_of_outputs() const;
 
      /** \brief Removes the outputs of this processor from storage */
      void flush_outputs();

      /** \brief Stops running processes and deactivates monitor */
      void shutdown();
  };

  std::istream& operator >> (std::istream&, processor::object_descriptor::t_status&);

  /**
   * \brief Basic monitor for task progress
   *
   * The process(es) that are spawned for this task are monitored via the
   * task_monitor interface.
   **/
  class processor::monitor : public execution::task_monitor {
    friend class processor_impl;

    public:

      /** \brief Type for functions that is used to handle incoming process state changes */
      typedef boost::function < void () >                                                     status_callback_function;

      /** \brief Type for functions that is used to handle incoming layout state changes */
      typedef boost::function < void (tipi::layout::tool_display::sptr) >                     display_layout_callback_function;

      /** \brief Type for functions that is used to handle incoming (G)UI state changes */
      typedef boost::function < void (tipi::layout::tool_display::constant_elements const&) > display_update_callback_function;

      /** \brief Type for functions that is used to handle incoming layout state changes */
      typedef boost::function < void (tipi::report::sptr) >                                   status_message_callback_function;

      /** \brief Convenience type for hiding shared pointer implementation */
      typedef boost::shared_ptr < monitor >                                                  sptr;

    public:

      /** \brief The processor that owns this object */
      processor& owner;

    private:
 
      /** \brief Actualisation function for process state changes */
      status_callback_function         status_change_handler;
 
    private:
 
      /** \brief Handler function that is called when an associated process changes state */
      void signal_change(const execution::process::status);

      /** \brief Handler function that is called when an associated process changes state */
      void signal_change(boost::shared_ptr < execution::process >, const execution::process::status);

      /** \brief Helper function for communication with a tool, starts a new thread with tool_configuration() */
      void start_tool_configuration(processor::sptr const&, boost::shared_ptr < tipi::configuration > const& c);

      /** \brief Helper function for communication with a tool, starts a new thread with tool_operation() */
      void start_tool_operation(processor::sptr const&, boost::shared_ptr < tipi::configuration > const&);

      /** \brief Actual tool configuration operation */
      void tool_configuration(processor::sptr, boost::shared_ptr < tipi::configuration >);

      /** \brief Actual tool execution with a configuration */
      void tool_operation(processor::sptr, boost::shared_ptr < tipi::configuration > const&);

    public:
 
      /** \brief Constructor with a callback handler */
      monitor(processor&);

      /** \brief Set the callback handler for status changes */
      void set_status_handler(status_callback_function);

      /** \brief Set the callback handler for display layout changes */
      void set_display_layout_handling(display_layout_callback_function const&, display_update_callback_function const&);

      /** \brief Set the callback handler for incoming status messages */
      void set_status_message_handler(status_message_callback_function);

      /** \brief Set the callback handler for display layout changes to default */
      void reset_display_layout_handling();

      /** \brief Set the callback handler for incoming status messages */
      void reset_status_message_handler();

      /** \brief Set all callback handlers to default */
      void reset_handlers();

      /** \brief Gets the status of the associated process, if there is one */
      execution::process::status get_status();
  };
}

#endif

