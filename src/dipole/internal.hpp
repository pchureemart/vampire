//------------------------------------------------------------------------------
//
//   This file is part of the VAMPIRE open source package under the
//   Free BSD licence (see licence file for details).
//
//   (c) Andrea Meo and Richard F L Evans 2016. All rights reserved.
//
//------------------------------------------------------------------------------
//

#ifndef DIPOLE_INTERNAL_H_
#define DIPOLE_INTERNAL_H_
//
//---------------------------------------------------------------------
// This header file defines shared internal data structures and
// functions for the dipole module. These functions and
// variables should not be accessed outside of this module.
//---------------------------------------------------------------------

// C++ standard library headers
#include <vector>

// Vampire headers
#include "dipole.hpp"

// dipole module headers
#include "internal.hpp"

namespace dipole{

   extern int update_rate; /// timesteps between updates
   extern void calculate_field();

   namespace internal{

      //-------------------------------------------------------------------------
      // Internal data type definitions
      //-------------------------------------------------------------------------

      //-------------------------------------------------------------------------
      // Internal shared variables
      //-------------------------------------------------------------------------
      extern bool initialised;

      extern int update_time; /// last update time

      extern const double prefactor; // 1e-7/1e30

      extern std::vector <std::vector < double > > rij_inter_xx;
      extern std::vector <std::vector < double > > rij_inter_xy;
      extern std::vector <std::vector < double > > rij_inter_xz;

      extern std::vector <std::vector < double > > rij_inter_yy;
      extern std::vector <std::vector < double > > rij_inter_yz;
      extern std::vector <std::vector < double > > rij_inter_zz;

      extern std::vector <std::vector < double > > rij_intra_xx;
      extern std::vector <std::vector < double > > rij_intra_xy;
      extern std::vector <std::vector < double > > rij_intra_xz;

      extern std::vector <std::vector < double > > rij_intra_yy;
      extern std::vector <std::vector < double > > rij_intra_yz;
      extern std::vector <std::vector < double > > rij_intra_zz;

      extern int num_atoms;
      extern std::vector < int > atom_type_array;
      extern std::vector < int > atom_cell_array;

      extern int cells_num_cells;
      extern int cells_num_local_cells;
      extern std::vector <int>  cells_local_cell_array;
      extern std::vector <int>  cells_num_atoms_in_cell;
      extern std::vector < double > cells_volume_array;

      //-------------------------------------------------------------------------
      // Internal function declarations
      //-------------------------------------------------------------------------
      //void write_macrocell_data();
      extern void update_field();

   } // end of internal namespace

} // end of dipole namespace

#endif //DIPOLE_INTERNAL_H_
