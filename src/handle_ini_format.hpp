#ifndef HANDLEFORMAT_H
#define HANDLEFORMAT_H

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <sstream>
#include <set>
#include <sys/stat.h>

extern std::string config_dir_and_file;   //Trans-file global variable holding the (path + config file name) combined string

const std::string semicolon_symb = ";";
const std::string hash_symb = "#";

class HandleFormat
{
public:

    /*Designate ini file name*/
	void set_ini_name(const std::string ini_file_name);

    /*Current file path*/
    std::string fetch_pwd(const std::string filename, const std::string binname);

    /*Read ini file*/
	bool scan_ini_f();

    /*Modify word in a given line*/
	bool modify_format();

    /*Write out config file lines*/
    bool create_config();

private:

    /*Replace word in a line*/
    bool substitute_string(std::string& ln_string, const std::string& inp_string, const std::string new_str);

    /*Check for folder*/
    const bool folder_found(const std::string inp_string);

    /*Check if file exists*/
    const bool file_dir_found(const std::string inp_string);

    /*Get file path*/
	std::string fetch_f_dir(const std::string inp_string);

    /*Fetch only file name sans the extension*/
    std::string fetch_f_name(const std::string inp_string, const std::set<char> delimiters);

private:
	std::string					input_file_name;
	std::vector<std::string>	line_to_parse;
};

#endif
