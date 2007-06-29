// Author(s): Bas Ploeger and Carst Tankink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file simreader.h
/// \brief Add your file description here.

#ifndef SIMREADER_H
#define SIMREADER_H
#include "simulation.h"

class simReader {
  public: 
    simReader(Simulation* s);
    virtual ~simReader();
    virtual void refresh() = 0; 
    virtual void selChange() = 0;
    virtual void setSim(Simulation* s);
 
  protected:
    Simulation* sim;

  private:
    Simulation::simConnection connection;
    Simulation::simConnection chooseConnection;
};

#endif

