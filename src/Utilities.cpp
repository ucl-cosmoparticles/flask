/***************************************************************
2012-09-18: Utility functions: error handling. 
***************************************************************/

#include "Utilities.hpp"
#include <iostream>
#include <cstdlib>   // to use exit()
#include <iomanip>   // for setw()
#include <ctime>


// Print some stats before finish the code run: 
void PrepareEnd(time_t StartAll) {
  double TotalTime;
  int min, sec;

  TotalTime = difftime(time(NULL), StartAll);
  min       = ((int)TotalTime)/60;
  sec       = ((int)TotalTime)%60;
  printf("\nTotal running time:       %d min, %d sec.\n", min, sec);
  printf(  "Total number of warnings: %d\n\n", warning("count"));
}


/*** Find out number of columns and rows in file ***/
void CountEntries(std::string filename, long *nr, long *nc) {
  using std::ifstream;
  using std::string;
  using std::istringstream;
  using std::ostringstream;
  long nrows=0, ncols=0;
  int nheaders=0;
  ifstream file;
  istringstream inputline;
  string word, phrase;
  
  // Open file
  file.open(filename.c_str());
  if (!file.is_open()) error("CountEntries: cannot open file.");
   
  // Detect headers (must start with # or empty lines):
  getline(file,phrase);
  while(!file.eof() && (phrase[0]=='#' || phrase.length()==0)) {nheaders++; getline(file,phrase);}
  // Count number of columns (using first data row):
  inputline.str(phrase);
  while (inputline >> word) ncols++;
  // Count number of rows (ignoring comments and empty spaces):
  while(!file.eof() && phrase[0]!='#' && phrase.length()!=0) {getline(file,phrase); nrows++;}
  if (phrase.length()!=0 && phrase[0]!='#') nrows++;

  file.close();
  *nr=nrows;
  *nc=ncols;
}


/****** Replace substring 'from' to 'to' in string 'str' ******/
bool StrReplace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos) return false;
    str.replace(start_pos, from.length(), to);
    return true;
}


/****** Print message and measure time elapsed from message printing to next function call with empty parameters ******/
void Announce(std::string message) {
  static time_t start;
  int space=58;
  double diff;

  // Announce the beginning of a process:
  if (message!="done") {
    //std::cout.width(space);
    std::cout << std::left << std::setw(space) << message << std::right; std::cout.flush();
    start = time(NULL);
  }

  // Announce the end of a process: 
  else {
    diff = difftime(time(NULL), start);
    printf("done.  (%gs)\n", diff);
  }
}

/****** Import vertical vectors from file ******/
// No memory allocation is done here. It must be done before.  
void ImportVecs(double **matriz, long length, long nvecs, const std::string & filename) {
  using std::ifstream;
  using std::string;
  using std::istringstream;
  using std::ostringstream;
  long nrows=0, ncols=0, i, j, nheaders=0;
  ifstream file;
  istringstream inputline; ostringstream outputline;
  string word, phrase;
  int datapos=0;
  
  // Open file
  file.open(filename.c_str());
  if (!file.is_open()) error("ImportVecs: cannot open file "+filename);
  
  // Detect headers (must start with # or empty lines):
  getline(file,phrase);
  while(!file.eof() && (phrase[0]=='#' || phrase.length()==0)) {datapos=file.tellg(); nheaders++; getline(file,phrase);}
  // Count number of columns (using first data row):
  outputline << phrase;
  inputline.str(outputline.str());
  while (inputline >> word) ncols++;
  if (ncols!=nvecs) error("ImportVecs: did not find expected number of columns.");
  
  // Rewind file start of data:
  file.clear();
  file.seekg(datapos);
  
  // Read data into vectors:
  for (i=0; i<length; i++)
    for (j=0; j<nvecs; j++) 
      if (!(file >> matriz[j][i])) error("ImportVecs: more data expected in file "+filename); 
                                   // DO NOT put comments in the end of the file!
  if(file >> word && word[0]!='#') error("LoadVecs: data was ignored in "+filename);

  file.close();
}


// Read column names in a file. It assumes the header are lines in the beginning of the file
// that start with #, and that the columns are in the last line of the header, separated 
// by spaces, e.g.: l Cl-f1z1f1z1 Cl-f1z2f1z2 Cl-f1z1f1z2.
// This function ALLOCATES MEMORY for array of column names and returns number of them.
long GetColumnNames(std::string filename, std::string **ColumnNames, int verbose) {
  using std::ifstream;
  using std::string;
  using std::istringstream;
  long ncols=0, i;
  ifstream file;
  istringstream inputline;
  string word, phrase, header;
  
  // Open file
  file.open(filename.c_str());
  if (!file.is_open()) error("GetColumnNames: cannot open file "+filename);
  
  // Detect headers (must start with # or empty lines) and select last one as column names:
  getline(file,phrase);
  while(!file.eof() && (phrase[0]=='#' || phrase.length()==0)) { 
    if (phrase[0]=='#') header.assign(phrase); 
    getline(file,phrase); 
  }

  // Remove all # before column names:
  i=1;
  while (header[i]=='#') i++;
  header.assign(header.substr(i,header.length()));

  // Count number of columns (using first data row):
  inputline.str(header);
  while (inputline >> word) ncols++;

  // Allocate memory for column names and parse them:
  if (verbose==1) std::cout << "GetColumnNames will allocate "<<ncols<<" strings in a vector.\n";
  *ColumnNames = vector<std::string>(0,ncols-1);
  inputline.clear(); inputline.seekg(0);
  for (i=0; i<ncols; i++) inputline >> (*ColumnNames)[i];

  return ncols;
}


// Error handling functions:
void error (const std::string message) {
  //using std::cout;
  using std::cerr;
  //cout << "\n!! ERROR!  " << message << " !!\n";
  cerr << "\n!! ERROR!  " << message << " !!\n";
  exit(1);
}
int warning (const std::string message) {
  using std::cout;
  using std::cerr;
  static int counter=0;
  if (message!="count") {
    //cout << "\n!! WARNING!  " << message << " !!\n";
    cerr << "\n!! WARNING!  " << message << " !!\n";
    counter++;
    return 0;
  }
  return counter;
}


/*** Pads a string containing a number with zeroes on the left ***/
std::string ZeroPad(int num, int max) {
  std::stringstream ss;
  int ndigits=1;
  
  while (max >= 10) {max=max/10; ndigits++;}
  
  
  ss << std::setfill('0') << std::setw(ndigits) << num;
  return ss.str();
}


// Check if a string is a natural number:
bool IsNumber(std::string str) {
  int i=0;
  while (i<str.length() && isdigit(str[i])!=0) i++;
  return (i==str.length()); 
}


// Convert string to integer:
int str2int(std::string str) {
  std::stringstream ss(str);
  int x;
  ss >> x;
  return x;
}


// Print header to file:
void PrintHeader(std::string *labels, int Nentries, std::ostream *output) {
  int i;
  *output << "# ";
  for (i=0; i<Nentries; i++) {
    if (labels[i].size()>0) *output << labels[i] << " ";
    else *output << "EmptyCol" << " ";
  }
  *output << std::endl;
}
