//------------------------------------------------------------------------------
//
//   This file is part of the VAMPIRE open source package under the
//   Free BSD licence (see licence file for details).
//
//   (c) Andrea Meo 2016. All rights reserved.
//
//   Email: am1808@york.ac.uk
//
//------------------------------------------------------------------------------

// C++ standard library headers
#include <cmath>
#include <algorithm>
#include <vector>

// Vampire headers
#include "cells.hpp"
#include "random.hpp"
#include "errors.hpp"
#include "vmpi.hpp"
#include "create.hpp"

#include "atoms.hpp"

// Local temperature pulse headers
#include "internal.hpp"

namespace cells{

   //-----------------------------------------------------------------------------
   // Function for calculate magnetisation in cells
   //-----------------------------------------------------------------------------
   //int mag(const double time_from_start){
   int mag(){

     // check calling of routine if error checking is activated
      if(err::check==true) std::cout << "cells::mag has been called" << std::endl;

      for(int i=0; i<cells::num_cells; ++i) {
         cells::mag_array_x[i] = 0.0;
         cells::mag_array_y[i] = 0.0;
         cells::mag_array_z[i] = 0.0;
      }

      #ifdef MPICF
         int num_local_atoms = vmpi::num_core_atoms+vmpi::num_bdry_atoms;
      #else
         int num_local_atoms = cells::internal::num_atoms;
      #endif
      //std::cout << "num_local_atoms from mag.cpp = " << num_local_atoms << std::endl;

      // calulate total moment in each cell
      for(int i=0;i<num_local_atoms;++i) {
         int cell = cells::atom_cell_array[i];
         int type = cells::internal::atom_type_array[i];
         // Consider only cells with n_atoms != 0
         if(cells::num_atoms_in_cell[cell]>0){
            const double mus = mp::material[type].mu_s_SI;
            // Consider only magnetic elements
            if(mus/(9.274e-24) > 0.5){
               cells::mag_array_x[cell] += atoms::x_spin_array[i]*mus;
               cells::mag_array_y[cell] += atoms::y_spin_array[i]*mus;
               cells::mag_array_z[cell] += atoms::z_spin_array[i]*mus;
            }
         }
      }

      #ifdef MPICF
      // Reduce magnetisation on all nodes
      MPI::COMM_WORLD.Allreduce(MPI_IN_PLACE,&cells::mag_array_x[0],cells::num_cells,MPI_DOUBLE,MPI_SUM);
      MPI::COMM_WORLD.Allreduce(MPI_IN_PLACE,&cells::mag_array_y[0],cells::num_cells,MPI_DOUBLE,MPI_SUM);
      MPI::COMM_WORLD.Allreduce(MPI_IN_PLACE,&cells::mag_array_z[0],cells::num_cells,MPI_DOUBLE,MPI_SUM);
      #endif

      return EXIT_SUCCESS;

   }
} // end of cells namespace
