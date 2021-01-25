#ifndef SELECTIONFUNC_H    // include guard.
#define SELECTIONFUNC_H 1

#include "definitions.hpp"
#include "ParameterList.hpp"

#ifndef FLASK_NO_HEALPIX_CXX_INCLUDE_DIR
#include <healpix_cxx/healpix_map.h>
#else
#include <healpix_map.h>
#endif

#include <gsl/gsl_randist.h>    // Random numbers.
#include "FieldsDatabase.hpp"

// SelectionFunction class interface:
class SelectionFunction {
private:
  int NoMap;
  Healpix_Map<SEL_PRECISION> *AngularSel;
  Healpix_Map<SEL_PRECISION>  StarMask;
  double **zSel, **zEntries, **fieldZrange, zSearchTol, Scale, *intZsel;
  long *NzEntries;
  int Separable, Nfields, *ftype, Npixels, N1, N2, *tracerIndex, NgalTypes, SelectionType, UseStarMask, UseAngularMask;
  int IndexGalTypes(const FZdatabase & fieldlist);
  bool yesShearSel;
public:
  SelectionFunction();
  void load(const ParameterList & config, const FZdatabase & fieldlist);
  int Nside() const;
  int Scheme() const;
  double operator()(int fz, int pix);
  double operator()(int fz);
  double RandRedshift(gsl_rng *r, int fz, int pix);
  int MaskBit(int fz, int pix) const;
  ~SelectionFunction();
};


// Other functions, not members but related:
void SelectionMemTest1(const ParameterList & config, int *ftype0, double **fzrange, int N10, int N20);
#endif
