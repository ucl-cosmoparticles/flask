#ifndef FITSFUNCTIONS_H
#define FITSFUNCTIONS_H 1

#include "definitions.hpp"
#include "ParameterList.hpp"

#ifndef FLASK_NO_HEALPIX_CXX_INCLUDE_DIR
#include <healpix_cxx/arr.h>
#else
#include <arr.h>
#endif

 
int WriteCatalog2Fits(std::string filename, CAT_PRECISION **table, long Nentries, const ParameterList & config, std::string headerline, bool float32bit);
int ReadHealpixData(int col, const ParameterList & config, double *weights, int datatype);

#endif
