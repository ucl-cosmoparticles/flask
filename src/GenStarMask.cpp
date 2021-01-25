#include <iostream>

#ifndef FLASK_NO_HEALPIX_CXX_INCLUDE_DIR
#include <healpix_cxx/healpix_map.h>
#include <healpix_cxx/healpix_map_fitsio.h>
#include <healpix_cxx/rangeset.h>
#else
#include <healpix_map.h>
#include <healpix_map_fitsio.h>
#include <rangeset.h>
#endif

#include <gsl/gsl_randist.h>    // Random numbers.
#include <cmath>


int main (int argc, char *argv[]) {
  // Input variables:
  int rnseed, Nside;
  double rmin, rmax, fracGoal; // rmin and rmax are given in arcmin.
  std::string filename;
  // Internal variables:
  double LNrmax, LNrmin, DLNr, r;
  Healpix_Map<double> StarMask;
  gsl_rng *rand;
  int Npixels, cpix, pixGoal, covPix, i;
  pointing cang;
  rangeset<int> pixSet;
  std::vector<int> starPixels;
  char message[200];
  // Constants:
  const double StarValue  = 0;
  const double EmptyValue = 1;

  // Check number of input parameters:
  if (argc<=6) {
    printf("USAGE: GenStarMask <SEED> <NSIDE> <RMIN> <RMAX> <FRAC> <FILENAME>\n");
    return 0;
  }
  // Read input parameters:
  sscanf(argv[1],"%d", &rnseed);
  sscanf(argv[2],"%d", &Nside);
  sscanf(argv[3],"%lf", &rmin);
  sscanf(argv[4],"%lf", &rmax);
  sscanf(argv[5],"%lf", &fracGoal);
  filename.assign(argv[6]);
  printf("Will generate Nside=%d map with %g frac. of stars with radius %g<r<%g arcmin using random seed %d:\n",
	 Nside, fracGoal, rmin, rmax, rnseed);
  

  // Setting random number generator:
  rand = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_set(rand, rnseed);

  // Initializing constants:
  LNrmin  = log(rmin * (M_PI/180.0/60.0)); // Convert arcmin to radians.
  LNrmax  = log(rmax * (M_PI/180.0/60.0)); // Convert arcmin to radians.
  DLNr    = LNrmax - LNrmin;
  Npixels = 12*Nside*Nside;

  // Initializing mask:
  StarMask.SetNside(Nside, RING);
  StarMask.fill(EmptyValue); 
 
  covPix  = 0;
  pixGoal = (int)(Npixels*fracGoal+0.5);
  // LOOP over stars:
  do {
    // Randomly select a center for a star:
    cpix = (int)(gsl_rng_uniform(rand)*Npixels);
    cang = StarMask.pix2ang(cpix);
    // Select disc of pixels around it:
    r    = exp(LNrmin + gsl_rng_uniform(rand)*DLNr);
    StarMask.query_disc(cang, r, pixSet);
    pixSet.toVector(starPixels);
    // Set selected pixels to zero and count zeroed pixels:
    for (i=0; i<starPixels.size(); i++) 
      if (StarMask[starPixels[i]]!=StarValue) {
	covPix++;
	StarMask[starPixels[i]] = StarValue;
      }   
  } while (covPix < pixGoal);
  
  // Output map to FITS file:
  write_Healpix_map_to_fits("!"+filename, StarMask, planckType<double>()); // Filename prefixed by ! to overwrite.
  printf("Map written to %s\n", filename.c_str());
  
  return 0;
}
