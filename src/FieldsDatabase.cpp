#include "Utilities.hpp"
#include "FieldsDatabase.hpp"

/************************************/
/*** Internal auxiliary functions ***/
/************************************/

// Return the position of entry in list or -1 of not found:
int FZdatabase::GetPos(int ent, std::vector<entry> & list) {
  int i;
  for (i=0; i<list.size(); i++) if (ent==list[i].IDnum) return i;
  return -1;
}


// Either create new entry in 'vec' or register it as another appearance of previous one:
void FZdatabase::RegisterEntry(int *full, int i, std::vector<entry> & vec, int **ixsuby) {
  int pos;
  entry aux;

  pos = GetPos(full[i], vec);
  // If entry is new, add to list of distinct values:
  if (pos==-1) {
    aux.IDnum    = full[i];
    aux.Ncopies  = 1;
    aux.index.resize(1); aux.index[0]=i;
    vec.push_back(aux);
    ixsuby[i][0] = vec.size()-1;
    ixsuby[i][1] = 0;
  }
  // If not, add the index to the entries list:
  else { 
    vec[pos].index.push_back(i); 
    vec[pos].Ncopies++; 
    ixsuby[i][0] = pos;
    ixsuby[i][1] = vec[pos].Ncopies-1;
  }
}


// Initialize an empty FZdatabase:
void FZdatabase::Init(int *fullF0, int *fullZ0, int Nfield0) {
  int i, j, pos;
  bool fDejavu, zDejavu;
  entry aux;

  // Allocate memory for field and redshift list from input:
  Nfield = Nfield0;
  fullF  = vector<int>(0, Nfield-1);
  fullZ  = vector<int>(0, Nfield-1);
  ifsubz = matrix<int>(0, Nfield-1, 0, 1);
  izsubf = matrix<int>(0, Nfield-1, 0, 1);
 
  // Copy list to internal memory, organizing in database:
  for (i=0; i<Nfield; i++) {
    fullF[i] = fullF0[i]; 
    fullZ[i] = fullZ0[i];
    RegisterEntry(fullF, i, f, ifsubz);
    RegisterEntry(fullZ, i, z, izsubf);
  }

  Nf = f.size();
  Nz = z.size();  
}


/************************/
/*** Public functions ***/
/************************/


// Empty constructor:
FZdatabase::FZdatabase() {
  Nf     = 0;
  Nz     = 0;
  Nfield = 0;  
}


// Constructor with input:
FZdatabase::FZdatabase(int *fullF0, int *fullZ0, int Nfield0) {
  Init(fullF0, fullZ0, Nfield0);
}


// Build fields and redshifts database erasing previous one:
void FZdatabase::Build(int *fullF0, int *fullZ0, int Nfield0) {
  int i, j, pos;
  bool fDejavu, zDejavu;
  entry aux;

  // Destroy previous allocation if existent:
  if (Nf>0) { free_vector(fullF, 0, Nf); f.resize(0); }
  if (Nz>0) { free_vector(fullZ, 0, Nz); z.resize(0); }

  // Initialize with new information:
  Init(fullF0, fullZ0, Nfield0);
}


// Destructor:
FZdatabase::~FZdatabase() {
  if (Nf>0) free_vector(fullF, 0, Nf);
  if (Nz>0) free_vector(fullZ, 0, Nz); 
  if (Nfield>0) free_matrix(ifsubz, 0, Nfield-1, 0, 1);
  if (Nfield>0) free_matrix(izsubf, 0, Nfield-1, 0, 1);
}


// Return number of field types:
int FZdatabase::Nfs() const {
  return Nf;
}


// Return number of redshift bins:
int FZdatabase::Nzs() const {
  return Nz;
}


// Return total number of Fields (Nf*Nz):
int FZdatabase::Nfields() const {
  return Nfield;
}


// Return number of redshift bins for a particular field index 'fi':
int FZdatabase::Nz4f(int fi) const {
  if (fi>Nf || fi<0) error("FZdatabase.Nz4f: requested field does not exist");
  return f[fi].Ncopies;
}


// Return number of fields found at a particular redshift bin index 'zi':
int FZdatabase::Nf4z(int zi) const {
  if (zi>Nz || zi<0) error("FZdatabase.Nf4z: requested redshift bin does not exist");
  return z[zi].Ncopies; 
}


// Given a Field index 'n', returns the field index 'fi' and the redshift sub index 'zi':
void FZdatabase::Index2fFixed(int n, int *fi, int *zi) const {
  *fi = ifsubz[n][0];
  *zi = ifsubz[n][1];
}


// Given a Field index 'n', returns the redshift index 'zi' and the field sub index 'fi':
void FZdatabase::Index2zFixed(int n, int *fi, int *zi) const {
  *fi = izsubf[n][1];
  *zi = izsubf[n][0];
}
  


// Return the index of the Field with z index 'zi' and f sub-index 'fi':
// (This is for LOOPS over f with z fixed)
int FZdatabase::zFixedIndex(int fi, int zi, int *n) const {
  if (zi<0 || zi>Nz)            error("FZdatabase.zFixedIndex: unknown redshift index");
  if (fi<0 || fi>z[zi].Ncopies) error("FZdatabase.zFixedIndex: unknown field sub-index");
  if (n!=NULL) *n = z[zi].index[fi]; 
  return z[zi].index[fi];
}


// Return the index of the Field with f index 'fi' and z sub-index 'zi':
// (This is for LOOPS over z with f fixed)
int FZdatabase::fFixedIndex(int fi, int zi, int *n) const {
  if (fi<0 || fi>Nf)            error("FZdatabase.fFixedIndex: unknown field index");
  if (zi<0 || zi>f[fi].Ncopies) error("FZdatabase.fFixedIndex: unknown redshift sub-index");
  if (n!=NULL) *n = f[fi].index[zi]; 
  return f[fi].index[zi];
}


// Return field NAME and redshift bin NAME for an index n:
void FZdatabase::Index2Name(int n, int *fName, int *zName) const {
  *fName = fullF[n];
  *zName = fullZ[n];
}


// Return index of a Field given a name: 
int FZdatabase::Name2Index(int fName, int zName, int *n) const {
  const int failed=-1;
  int i;

  for (i=0; i<Nfield; i++) 
    if (fName==fullF[i] && zName==fullZ[i]) {
      if (n!=NULL) *n = i;
      return i;
    }
  
  warning("FZdatabase.Name2Index: could not find requested field");  
  *n = failed;
  return failed;
}


// Return field NAME and redshift bin NAME for an z index 'zi' and f sub-index 'fi':
void FZdatabase::zFixedName(int fi, int zi, int *fName, int *zName) const {
  Index2Name(zFixedIndex(fi, zi), fName, zName);
}


// Return field NAME and redshift bin NAME for an f index 'fi' and z sub-index 'zi':
void FZdatabase::fFixedName(int fi, int zi, int *fName, int *zName) const {
  Index2Name(fFixedIndex(fi, zi), fName, zName);
}
