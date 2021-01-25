#ifndef GENERALOUTPUT     // include guard.
#define GENERALOUTPUT 1

#include "definitions.hpp" // For MAP and ALM _PRECISION. 

#ifndef FLASK_NO_HEALPIX_CXX_INCLUDE_DIR
#include <healpix_cxx/healpix_map.h>   // For RandAngInPix function.
#include <healpix_cxx/alm.h>           // For GeneralOutput function.
#include <healpix_cxx/xcomplex.h>      // For GeneralOutput function.
#else
#include <healpix_map.h>
#include <alm.h>
#include <xcomplex.h>
#endif

#include "ParameterList.hpp"
#include "Utilities.hpp"
#include <gsl/gsl_matrix.h> // For GSL matrix output.
#include "FieldsDatabase.hpp"

/*** Matrix output ***/

// Prints a GSL matrix to a file:
void GeneralOutput(const gsl_matrix *Cov, std::string filename, bool inform = 1);
// Prints all GSL matrices in a vector to files:
void GeneralOutput(gsl_matrix **CovByl, const ParameterList & config, std::string keyword, bool inform = 1);


/*** Cl's output ***/

// Prints all Cl's to a TEXT file:
void GeneralOutput(double **recovCl, bool *yesCl, const FZdatabase & fieldlist, 
		   const ParameterList & config, std::string keyword, bool inform = 1);


/*** Alm's output ***/

// Many alm's in one single table:
void GeneralOutput(Alm<xcomplex <ALM_PRECISION> > *af, const ParameterList & config, std::string keyword, 
		   const FZdatabase & fieldlist, bool inform = 1);
// One alm in one table, named with a prefix and the field ID:
void GeneralOutput(const Alm<xcomplex <ALM_PRECISION> > & a, const ParameterList & config, std::string keyword, int f, int z, bool inform = 1);
// One alm in one table, names with a keyword:
void GeneralOutput(const Alm<xcomplex <ALM_PRECISION> > & a, const ParameterList & config, std::string keyword, bool inform = 1);


/*** Healpix map output ***/

// Prints one single map to FITS file based on a PREFIX and a FIELD ID:
void GeneralOutput(const Healpix_Map<MAP_PRECISION> & map, const ParameterList & config, std::string keyword, int *fnz, bool inform = 1);
// Prints a list of maps to a single TEXT file:
void GeneralOutput(Healpix_Map<MAP_PRECISION> *mapf, const ParameterList & config, std::string keyword, 
		   const FZdatabase & fieldlist, bool fits=0, bool inform = 1);
// Prints two lists of maps to a single TEXT file:
void GeneralOutput(Healpix_Map<MAP_PRECISION> *gamma1, Healpix_Map<MAP_PRECISION> *gamma2, 
		   const ParameterList & config, std::string keyword, const FZdatabase & fieldlist, bool inform = 1);
// One set of (Kappa, gamma1, gamma2) maps to one FITS file, named with a prefix and the field ID:
void GeneralOutput(const Healpix_Map<MAP_PRECISION> & kmap, const Healpix_Map<MAP_PRECISION> & g1map, 
		   const Healpix_Map<MAP_PRECISION> & g2map, const ParameterList & config, std::string keyword, 
		   int f, int z, bool inform = 1);
// One set of (Kappa, gamma1, gamma2) maps to one FITS file, named with a keyword:
void GeneralOutput(const Healpix_Map<MAP_PRECISION> & kmap, const Healpix_Map<MAP_PRECISION> & g1map, 
		   const Healpix_Map<MAP_PRECISION> & g2map, const ParameterList & config, std::string keyword, bool inform = 1);
// One single map to one FITS file, named with a keyword:
void GeneralOutput(const Healpix_Map<MAP_PRECISION> & map, const ParameterList & config, std::string keyword, bool inform = 1);

#endif
