//-----------------------------------------------------------------------------
//
//  Vampire - A code for atomistic simulation of magnetic materials
//
//  Copyright (C) 2009-2012 R.F.L.Evans
//
//  Email:richard.evans@york.ac.uk
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// ----------------------------------------------------------------------------
//
///
/// @file
/// @brief Contains routines for outputting magnetisation snapshots
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2011. All Rights Reserved.
///
/// @section info File Information
/// @author  Richard Evans, richard.evans@york.ac.uk
/// @version 1.0
/// @date    30/05/2011
/// @internal
///	Created:		30/05/2011
///	Revision:	  ---
///=====================================================================================
///

// Standard Libraries
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


// Vampire Header files
#include "atoms.hpp"
#include "cells.hpp"
#include "dipole.hpp"
#include "errors.hpp"
#include "LLG.hpp"
#include "material.hpp"
#include "sim.hpp"
#include "stats.hpp"
#include "vio.hpp"
#include "vmpi.hpp"

namespace vout{

   bool output_atoms_config=false;
   int output_atoms_config_rate=1000;

   //output_rate_counter_defined globally => not to be redifined here!!

   double atoms_output_min[3]={0.0,0.0,0.0};
   double atoms_output_max[3]={1.0,1.0,1.0};
   double field_output_min_1=-10000.0;
   double field_output_max_1=-0.0;
   double field_output_min_2=0.0;
   double field_output_max_2=10000.0;
   int total_output_atoms=0;
   std::vector<int> local_output_atom_list(0);

   int output_rate_counter_coords=0;

   bool output_cells_config=false;
   int output_cells_config_rate=1000;

   bool output_grains_config=false;
   int output_config_grain_rate=1000;
   int output_grains_file_counter=0;

   // function headers
   void atoms();
   void atoms_coords();
   void cells();
   void cells_coords();

/// @brief Config master output function
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2011. All Rights Reserved.
///
/// @section Information
/// @author  Richard Evans, richard.evans@york.ac.uk
/// @version 1.0
/// @date    31/05/2011
///
/// @internal
///	Created:		31/05/2011
///	Revision:	  ---
///=====================================================================================
///
void config(){

   double minField_1;
   double maxField_1;
   double minField_2;
   double maxField_2;

   // check that minField_1>maxField_1
   if(vout::field_output_min_1>=vout::field_output_max_1){
      minField_1=vout::field_output_min_1;
      maxField_1=vout::field_output_max_1;
   }
   else{
      minField_1=vout::field_output_max_1;
      maxField_1=vout::field_output_min_1;
   }
   // check that maxField_2>minField_2
   if(vout::field_output_max_2>=vout::field_output_min_2){
      minField_2=vout::field_output_min_2;
      maxField_2=vout::field_output_max_2;
   }
   else{
      minField_2=vout::field_output_max_2;
      maxField_2=vout::field_output_min_2;
   }

   // check calling of routine if error checking is activated
   if(err::check==true){std::cout << "vout::config has been called" << std::endl;}

   // atoms output
   if((vout::output_atoms_config==true) && (sim::output_rate_counter%output_atoms_config_rate==0)){
      if(sim::program!=2){
         if(vout::output_rate_counter_coords==0) vout::atoms_coords();
         vout::atoms();
         vout::output_rate_counter_coords++;
      }
      else if(sim::program==2){
         // output config only in range [minField_1;maxField_1] for decreasing field
         if((sim::H_applied>=maxField_1) && (sim::H_applied<=minField_1) && (sim::parity<0)){
            if(vout::output_rate_counter_coords==0) vout::atoms_coords();
            vout::atoms();
            vout::output_rate_counter_coords++;
         }
	      // output config only in range [minField_2;maxField_2] for increasing field
         else if((sim::H_applied>=minField_2) && (sim::H_applied<=maxField_2) && (sim::parity>0)){
            if(vout::output_rate_counter_coords==0) vout::atoms_coords();
            vout::atoms();
            vout::output_rate_counter_coords++;
         }
      }
   }

   // cells output
   if((vout::output_cells_config==true) && (sim::output_rate_counter%output_cells_config_rate==0)){
      // if(!program::hysteresis())
      if(sim::program!=2){
         if(sim::output_cells_file_counter==0) vout::cells_coords();
         vout::cells();
      }
      else if(sim::program==2){
         // output config only in range [minField_1;maxField_1] for decreasing field
         if((sim::H_applied>=maxField_1) && (sim::H_applied<=minField_1) && (sim::parity<0)){
            if(sim::output_cells_file_counter==0) vout::cells_coords();
            vout::cells();
         }
         // output config only in range [minField_2;maxField_2] for increasing field
         else if((sim::H_applied>=minField_2) && (sim::H_applied<=maxField_2) && (sim::parity>0)){
            if(sim::output_cells_file_counter==0) vout::cells_coords();
            vout::cells();
         }
      }
   }

   // increment rate counter
   sim::output_rate_counter++;

}
/// @brief Atomistic output function
///
/// @details Outputs formatted data snapshot for visualisation
///
///	#------------------------------------------------------
///	# Atomistic spin configuration file for vampire
///	#------------------------------------------------------
///	# Date: xx/xx/xxxx xx.xx.xx
///	#------------------------------------------------------
///	Number of spins: $n_spins
///	System dimensions: $max_x $max_y $max_z
///	Coordinates-file: $coord_file
///	Time: $t
///	Field: $H
///	Temperature: $T
///	Magnetisation: $mx $my $mz
///	Number of Materials: $n_mat
///	Material Properties 1:	$mu_s	$mmx $mmy $mmz $mm
///	Material Properties 2:	$mu_s	$mmx $mmy $mmz ...
///	#------------------------------------------------------
///	Number of spin files: $n_files
///	atoms-000ID000-00CPU0.cfg
///	atoms-000ID000-00CPU1.cfg
///	atoms-000ID000-00CPU2.cfg
///	#------------------------------------------------------
///	Number of local spins: $n_loc_spins
///	$sx $sy $sz
///	$sx $sy ...
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2011. All Rights Reserved.
///
/// @section Information
/// @author  Richard Evans, richard.evans@york.ac.uk
/// @version 1.0
/// @date    30/05/2011
///
/// @internal
///	Created:		30/05/2011
///	Revision:	  ---
///=====================================================================================
///
   void atoms(){

      // check calling of routine if error checking is activated
      if(err::check==true){std::cout << "vout::atoms has been called" << std::endl;}

      #ifdef MPICF
         const int num_atoms = vmpi::num_core_atoms+vmpi::num_bdry_atoms;
      #else
         const int num_atoms = atoms::num_atoms;
      #endif

      // Set local output filename
      std::stringstream file_sstr;
      file_sstr << "atoms-";
      // Set CPUID on non-root process
      if(vmpi::my_rank!=0){
         file_sstr << std::setfill('0') << std::setw(5) << vmpi::my_rank << "-";
      }
      file_sstr << std::setfill('0') << std::setw(8) << sim::output_atoms_file_counter;
      file_sstr << ".cfg";
      std::string cfg_file = file_sstr.str();
      const char* cfg_filec = cfg_file.c_str();

      // Output informative message to log file
      zlog << zTs() << "Outputting configuration file " << cfg_file << " to disk" << std::endl;

      // Declare and open output file
      std::ofstream cfg_file_ofstr;
      cfg_file_ofstr.open (cfg_filec);

      // Output masterfile header on root process
      if(vmpi::my_rank==0){
         // Get system date
      time_t rawtime = time(NULL);
      struct tm * timeinfo = localtime(&rawtime);

         cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
         cfg_file_ofstr << "# Atomistic spin configuration file for vampire"<< std::endl;
         cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
         cfg_file_ofstr << "# Date: "<< asctime(timeinfo);
         cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
         cfg_file_ofstr << "Number of spins: "<< vout::total_output_atoms << std::endl;
         cfg_file_ofstr << "System dimensions:" << cs::system_dimensions[0] << "\t" << cs::system_dimensions[1] << "\t" << cs::system_dimensions[2] << std::endl;
         cfg_file_ofstr << "Coordinates-file: atoms-coord.cfg"<< std::endl;
         cfg_file_ofstr << "Time: " << double(sim::time)*mp::dt_SI << std::endl;
         cfg_file_ofstr << "Field: " << sim::H_applied << std::endl;
         cfg_file_ofstr << "Temperature: "<< sim::temperature << std::endl;
         cfg_file_ofstr << "Magnetisation: " << stats::system_magnetization.output_normalized_magnetization() << std::endl;
         cfg_file_ofstr << "Number of Materials: " << mp::num_materials << std::endl;
         for(int mat=0;mat<mp::num_materials;mat++){
            cfg_file_ofstr << mp::material[mat].mu_s_SI << std::endl;
         }
         cfg_file_ofstr << "#------------------------------------------------------" << std::endl;
         cfg_file_ofstr << "Number of spin files: " << vmpi::num_processors-1 << std::endl;
         for(int p=1;p<vmpi::num_processors;p++){
            std::stringstream cfg_sstr;
            cfg_sstr << "atoms-" << std::setfill('0') << std::setw(5) << p << "-" << std::setfill('0') << std::setw(8) << sim::output_atoms_file_counter << ".cfg";
            cfg_file_ofstr << cfg_sstr.str() << std::endl;
         }
         cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
      }

      // Everyone now outputs their atom list
      cfg_file_ofstr << vout::local_output_atom_list.size() << std::endl;
      for(int i=0; i<vout::local_output_atom_list.size(); i++){
         const int atom = vout::local_output_atom_list[i];
         cfg_file_ofstr << atoms::x_spin_array[atom] << "\t" << atoms::y_spin_array[atom] << "\t" << atoms::z_spin_array[atom] << std::endl;
      }

      cfg_file_ofstr.close();

      sim::output_atoms_file_counter++;

   }

/// @brief Atomistic output function
///
/// @details Outputs formatted data snapshot for visualisation
///
///	//------------------------------------------------------
///	// Atomistic coordinate configuration file for vampire
///	//------------------------------------------------------
///	// Date: xx/xx/xxxx xx.xx.xx
///	//------------------------------------------------------
///	Number of atoms: $n_spins
///	//------------------------------------------------------
///	Number of atom files: $n_files
///	atoms-coords-00CPU0.cfg
///	atoms-coords-00CPU1.cfg
///	atoms-coords-00CPU2.cfg
///	//------------------------------------------------------
///	Number of local atoms: $n_loc_atoms
///	$material $category $x $y $z $species
///	$material $category $x $y $z $species
///	$material $category $x ...
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2011. All Rights Reserved.
///
/// @section Information
/// @author  Richard Evans, richard.evans@york.ac.uk
/// @version 1.0
/// @date    31/05/2011
///
/// @internal
///	Created:		31/05/2011
///	Revision:	  ---
///=====================================================================================
///
   void atoms_coords(){

      // check calling of routine if error checking is activated
      if(err::check==true){std::cout << "vout::atoms_coords has been called" << std::endl;}

      #ifdef MPICF
         const int num_atoms = vmpi::num_core_atoms+vmpi::num_bdry_atoms;
      #else
         const int num_atoms = atoms::num_atoms;
      #endif

      // resize atom list to zero
      local_output_atom_list.resize(0);

      // get output bounds
      double minB[3]={vout::atoms_output_min[0]*cs::system_dimensions[0],
                     vout::atoms_output_min[1]*cs::system_dimensions[1],
                     vout::atoms_output_min[2]*cs::system_dimensions[2]};

      double maxB[3]={vout::atoms_output_max[0]*cs::system_dimensions[0],
                     vout::atoms_output_max[1]*cs::system_dimensions[1],
                     vout::atoms_output_max[2]*cs::system_dimensions[2]};

      // loop over all local atoms and record output list
      for(int atom=0;atom<num_atoms;atom++){

         const double cc[3] = {atoms::x_coord_array[atom],atoms::y_coord_array[atom],atoms::z_coord_array[atom]};

         // check atom within output bounds
         if((cc[0] >= minB[0]) && (cc[0]<=maxB[0])){
            if((cc[1] >= minB[1]) && (cc[1]<=maxB[1])){
               if((cc[2] >= minB[2]) && (cc[2]<=maxB[2])){
                  local_output_atom_list.push_back(atom);
               }
            }
         }
      }

      // calculate total atoms to output
      #ifdef MPICF
         int local_atoms = local_output_atom_list.size();
         int total_atoms;
         //std::cerr << vmpi::my_rank << "\t" << local_atoms << &local_atoms << "\t" << &total_atoms << std::endl;
         //MPI::COMM_WORLD.Barrier();
         MPI::COMM_WORLD.Allreduce(&local_atoms, &total_atoms,1, MPI_INT,MPI_SUM);
         vout::total_output_atoms=total_atoms;
         //std::cerr << vmpi::my_rank << "\t" << total_atoms << "\t" << &local_atoms << "\t" << &total_atoms << std::endl;
         //MPI::COMM_WORLD.Barrier();
      #else
         vout::total_output_atoms=local_output_atom_list.size();
      #endif

      // Set local output filename
      std::stringstream file_sstr;
      file_sstr << "atoms-coords";
      // Set CPUID on non-root process
      if(vmpi::my_rank!=0){
         file_sstr << "-" << std::setfill('0') << std::setw(5) << vmpi::my_rank;
      }
      file_sstr << ".cfg";
      std::string cfg_file = file_sstr.str();
      const char* cfg_filec = cfg_file.c_str();

      // Declare and open output file
      std::ofstream cfg_file_ofstr;
      cfg_file_ofstr.open (cfg_filec);

      // Output masterfile header on root process
      if(vmpi::my_rank==0){
         // Get system date
                  time_t rawtime = time(NULL);
      struct tm * timeinfo = localtime(&rawtime);

         cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
         cfg_file_ofstr << "# Atomistic coordinates configuration file for vampire"<< std::endl;
         cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
         cfg_file_ofstr << "# Date: "<< asctime(timeinfo);
         cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
         cfg_file_ofstr << "Number of atoms: "<< vout::total_output_atoms << std::endl;
         cfg_file_ofstr << "#------------------------------------------------------" << std::endl;
         cfg_file_ofstr << "Number of spin files: " << vmpi::num_processors-1 << std::endl;
         for(int p=1;p<vmpi::num_processors;p++){
            std::stringstream cfg_sstr;
            cfg_sstr << "atoms-coords-" << std::setfill('0') << std::setw(5) << p << ".cfg";
            cfg_file_ofstr << cfg_sstr.str() << std::endl;
         }
         cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
      }

      // Everyone now outputs their atom list
      cfg_file_ofstr << vout::local_output_atom_list.size() << std::endl;
      for(int i=0; i<vout::local_output_atom_list.size(); i++){
         const int atom = vout::local_output_atom_list[i];
         cfg_file_ofstr << atoms::type_array[atom] << "\t" << atoms::category_array[atom] << "\t" <<
         atoms::x_coord_array[atom] << "\t" << atoms::y_coord_array[atom] << "\t" << atoms::z_coord_array[atom] << "\t";
         if(sim::identify_surface_atoms==true && atoms::surface_array[atom]==true) cfg_file_ofstr << "O " << std::endl;
         else cfg_file_ofstr << mp::material[atoms::type_array[atom]].element << std::endl;
      }

      cfg_file_ofstr.close();

   }

/// @brief Cell output function
///
/// @details Outputs formatted data snapshot for visualisation
///
///   #------------------------------------------------------
///   # Cell configuration file for vampire
///   #------------------------------------------------------
///   # Date: xx/xx/xxxx xx.xx.xx
///   #------------------------------------------------------
///   Number of cells: $n_cells
///   System dimensions: $max_x $max_y $max_z
///   Coordinates-file: $coord_file
///   Time: $t
///   Field: $H
///   Temperature: $T
///   Magnetisation: $mx $my $mz
///   Number of Materials: $n_mat
///   Material Properties 1:  $mu_s $mmx $mmy $mmz $mm
///   Material Properties 2:  $mu_s $mmx $mmy $mmz ...
///   #------------------------------------------------------
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2011. All Rights Reserved.
///
/// @section Information
/// @author  Richard Evans, richard.evans@york.ac.uk
/// @version 1.0
/// @date    26/04/2013
///
/// @internal
///   Created:    26/04/2013
///   Revision:     ---
///=====================================================================================
///
void cells(){

   // check calling of routine if error checking is activated
   if(err::check==true){std::cout << "vout::cells has been called" << std::endl;}

   // Set local output filename
   std::stringstream file_sstr;
   file_sstr << "cells-";
   file_sstr << std::setfill('0') << std::setw(8) << sim::output_cells_file_counter;
   file_sstr << ".cfg";
   std::string cfg_file = file_sstr.str();
   const char* cfg_filec = cfg_file.c_str();

   #ifdef MPICF
   // Reduce demagnetisation fields to processor 0
   if(vmpi::my_rank==0){
      MPI_Reduce(MPI_IN_PLACE, &dipole::cells_field_array_x[0], cells::num_cells, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
      MPI_Reduce(MPI_IN_PLACE, &dipole::cells_field_array_y[0], cells::num_cells, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
      MPI_Reduce(MPI_IN_PLACE, &dipole::cells_field_array_z[0], cells::num_cells, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
   }
   else{
      MPI_Reduce(&dipole::cells_field_array_x[0], &dipole::cells_field_array_x[0], cells::num_cells, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
      MPI_Reduce(&dipole::cells_field_array_y[0], &dipole::cells_field_array_y[0], cells::num_cells, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
      MPI_Reduce(&dipole::cells_field_array_z[0], &dipole::cells_field_array_z[0], cells::num_cells, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
   }
   #endif

   // Output masterfile header on root process
   if(vmpi::my_rank==0){

      zlog << zTs() << "Outputting cell configuration " << sim::output_cells_file_counter << " to disk." << std::endl;

      // Declare and open output file
      std::ofstream cfg_file_ofstr;
      cfg_file_ofstr.open (cfg_filec);

      // Get system date
      time_t rawtime = time(NULL);
      struct tm * timeinfo = localtime(&rawtime);

      cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
      cfg_file_ofstr << "# Cell configuration file for vampire"<< std::endl;
      cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
      cfg_file_ofstr << "# Date: "<< asctime(timeinfo);
      cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
      cfg_file_ofstr << "# Number of spins: "<< cells::num_local_cells << std::endl;
      cfg_file_ofstr << "# System dimensions:" << cs::system_dimensions[0] << "\t" << cs::system_dimensions[1] << "\t" << cs::system_dimensions[2] << std::endl;
      cfg_file_ofstr << "# Coordinates-file: cells-coord.cfg"<< std::endl;
      cfg_file_ofstr << "# Time: " << double(sim::time)*mp::dt_SI << std::endl;
      cfg_file_ofstr << "# Field: " << sim::H_applied << std::endl;
      cfg_file_ofstr << "# Temperature: "<< sim::temperature << std::endl;
      cfg_file_ofstr << "# Magnetisation: " << stats::system_magnetization.output_normalized_magnetization() << std::endl;
      cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;

      // Root process now outputs the cell magnetisations
      for(int cell=0; cell < cells::num_cells; cell++){
         if(cells::num_atoms_in_cell[cell]>0){
            cfg_file_ofstr << cells::mag_array_x[cell] << "\t" << cells::mag_array_y[cell] << "\t" << cells::mag_array_z[cell]<< "\t";
            cfg_file_ofstr << dipole::cells_field_array_x[cell] << "\t" << dipole::cells_field_array_y[cell] << "\t" << dipole::cells_field_array_z[cell] << std::endl;
         }
      }

      cfg_file_ofstr.close();

   }

   sim::output_cells_file_counter++;

   return;

}

/// @brief Cells output function
///
/// @details Outputs formatted data snapshot for visualisation
///
///   //------------------------------------------------------
///   // Atomistic coordinate configuration file for vampire
///   //------------------------------------------------------
///   // Date: xx/xx/xxxx xx.xx.xx
///   //------------------------------------------------------
///   Number of cells: $n_cells
///   //------------------------------------------------------
///   Number of atom files: $n_files
///   atoms-coords-00CPU0.cfg
///   atoms-coords-00CPU1.cfg
///   atoms-coords-00CPU2.cfg
///   //------------------------------------------------------
///   Number of local cells: $n_loc_cells
///   $material $category $x $y $z $species
///   $material $category $x $y $z $species
///   $material $category $x ...
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2011. All Rights Reserved.
///
/// @section Information
/// @author  Richard Evans, richard.evans@york.ac.uk
/// @version 1.0
/// @date    31/05/2011
///
/// @internal
///   Created:    31/05/2011
///   Revision:     ---
///=====================================================================================
///
void cells_coords(){

   // check calling of routine if error checking is activated
   if(err::check==true){std::cout << "vout::atoms_coords has been called" << std::endl;}

   // Set local output filename
   std::stringstream file_sstr;
   file_sstr << "cells-coords";
   file_sstr << ".cfg";
   std::string cfg_file = file_sstr.str();
   const char* cfg_filec = cfg_file.c_str();

   // Output masterfile header on root process
   if(vmpi::my_rank==0){

      std::cout << "Outputting cell coordinates to disk." << std::endl;
      zlog << zTs() << "Outputting cell coordinates to disk." << std::endl;

      // Declare and open output file
      std::ofstream cfg_file_ofstr;
      cfg_file_ofstr.open (cfg_filec);

      // Get system date
      time_t rawtime = time(NULL);
      struct tm * timeinfo = localtime(&rawtime);

      cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
      cfg_file_ofstr << "# Cell coordinates configuration file for vampire"<< std::endl;
      cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
      cfg_file_ofstr << "# Date: "<< asctime(timeinfo);
      cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;
      cfg_file_ofstr << "# Number of cells: "<< cells::num_local_cells << std::endl;
      cfg_file_ofstr << "#------------------------------------------------------" << std::endl;
      cfg_file_ofstr << "#" << std::endl;
      cfg_file_ofstr << "#" << std::endl;
      cfg_file_ofstr << "#" << std::endl;
      cfg_file_ofstr << "#" << std::endl;
      cfg_file_ofstr << "#" << std::endl;
      cfg_file_ofstr << "#------------------------------------------------------"<< std::endl;

      for(int cell=0; cell<cells::num_cells; cell++){
         if(cells::num_atoms_in_cell[cell]>0){
            cfg_file_ofstr << cell << "\t" << cells::num_atoms_in_cell[cell] << "\t" << cells::cell_coords_array_x[cell] << "\t" << cells::cell_coords_array_y[cell] << "\t" << cells::cell_coords_array_z[cell] << std::endl;
         }
      }

      cfg_file_ofstr.close();
   }

   return;

}

} // End of namespace vout
