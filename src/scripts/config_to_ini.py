#!/usr/bin/python3

"""
USAGE:   config_to_ini.py <CONFIG_FILE> <INI_FILE> 
EXAMPLE: config_to_ini.py  input.config  output.ini

This script converts a config file used by Flask into an INI (.ini) file format
that is widely used in Microsoft Windows, PHP etc.
It is assumed that a comment in .ini file begins with a semicolon (;),
while section headers are wrapped inside squared brackets ( [ and ] ).

In this version, if a trailing comment is found on a line in the CONFIG file,
the line is split into two lines, with the first line printing out the comment
and the second line printing the variable that the comment describes. 

Author: Jayesh S. Bhatt
Email: jayesh.bhatt@ucl.ac.uk, bhatt@physics.org
"""

import sys
import argparse

numargs = len(sys.argv)

if numargs < 2:
    print ("ERROR: Input (config) and output (ini) filenames must be provided at the command line")
    print ("       E.g. config_to_ini.py  input.config  output.ini")
    exit()

inputfile  = sys.argv[1]  #First argument from the command line
outputfile = sys.argv[2]  #First argument from the command line



parser = argparse.ArgumentParser(description='Convert Flask CONFIG file to INI format.')



# Function to split a line into a list of individual characters
def splitchars(charstring): 
    return [char for char in charstring]

# Function to convert a list of single characters into a line:
def converttoline(charlist): 

    # initialization of string to "" 
    updatedstring = "" 
    # go through eaxh character in the string  
    for ch in charlist: 
        updatedstring += ch
  
    # return string  
    return updatedstring


configfile = open(inputfile,"r")
inifile = open(outputfile,"w")


for line in configfile:
    line_unstripped = line
    line = line.strip()      #Remove any junk in the line
    column = line.split()    #Split the line into individul words
    ncol = len(column)

    #Treat the section headers:
    if ncol > 0 and column[0] == '###' and column[ncol-1] == '###':
        #print ("Header name: ", column[1:ncol-1])
        headerstr = ' '.join(column[1:ncol-1])
        inifile.write("[" + headerstr + "]" + "\n")
    #Treat all the other lines:
    else:
        #Treat the comment lines
        charsinline = splitchars(line_unstripped)         # Split the entire line into individual characters
        nchar = len(charsinline)               # Work out how many characters are there in this line

        for ichar in range(0,nchar):
            if charsinline[ichar] == '#':
                if ichar <= 1:
                    charsinline[ichar] = ';'
                    line_modified = converttoline(charsinline) #+ "\n"
                    line_unstripped = converttoline(charsinline) #+ "\n"
                    ### inifile.write(line_modified)
                    break
                else:
                    charsinline[ichar] = ';'
                    charlist_1 = charsinline[ichar:nchar-1]
                    line_1 = converttoline(charlist_1) + "\n"  #Put the part that follows a hash (#) character as the 'upper' line

                    charlist_2 = charsinline[0:ichar-1]
                    line_2 = converttoline(charlist_2) + "\n"     #Put the part that precedes a hash (#) character as the 'lower' line

                    line_unstripped = line_1 + line_2
                    ### inifile.write(line_1)
                    ### inifile.write(line_2)
                    ### inifile.write("\n")
                    break
                    #for jchar in range(ichar,nchar-1):
                    #    charsinline[jchar] = ''
                    #break

        inifile.write(line_unstripped)


configfile.close()
inifile.close()
