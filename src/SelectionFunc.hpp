#ifndef SELECTIONFUNC_H    // include guard.
#define SELECTIONFUNC_H 1

#include "definitions.hpp"
#include "ParameterList.hpp"
#include <healpix_map.h>
#include <random>               // Random numbers.
#include "Utilities.hpp"
#include "FieldsDatabase.hpp"
#include "interpol.hpp"
#include "Maximize.hpp"

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
  double operator()(int fz, int pix) const;

  template<class RNG>
  double RandRedshift(RNG& rng, int fz, int pix);

  int MaskBit(int fz, int pix) const;
  ~SelectionFunction();
};


// Other functions, not members but related:
void SelectionMemTest1(const ParameterList & config, int *ftype0, double **fzrange, int N10, int N20);


// Returns random redshift for a field-redshift-bin 'fz' and pixel 'pix'
// according to the selection function.
// 'zSearchTol' is the tolerance for z position when looking for the Selection
// function maximum.
template<typename RNG>
double SelectionFunction::RandRedshift(RNG& rng, int fz, int pix) {
  double ymax, yguess, zguess, zBinSize;

  zBinSize = fieldZrange[fz][1] - fieldZrange[fz][0];

  std::uniform_real_distribution<> unif;

  // We should interpolate between selection function in the same pixel.
  if (Separable==0) { 
    error("SelectionFunction.RandRedshift: "
          "not yet implemented for non-separable selection functions.");
    zguess = 0;
  }
  else if (Separable==1 || Separable==2) {
    ymax = MaxInterp(fieldZrange[fz][0], fieldZrange[fz][1], zSearchTol, 
                     zEntries[tracerIndex[fz]], NzEntries[tracerIndex[fz]],
                     zSel[tracerIndex[fz]]);
    do {
      zguess = fieldZrange[fz][0] + unif(rng)*zBinSize;
      yguess = unif(rng)*ymax;
    } while (yguess > Interpol(zEntries[tracerIndex[fz]],
                               NzEntries[tracerIndex[fz]],
                               zSel[tracerIndex[fz]], zguess));

  }

  return zguess;
}

#endif
