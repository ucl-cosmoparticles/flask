#include "handle_ini_format.hpp"

std::string config_dir_and_file;   // This is a global variable

/*******************************************************************
Designate ini file name
*******************************************************************/
void HandleFormat::set_ini_name(const std::string ini_file_name)
{
	input_file_name = ini_file_name;
}

/***********************************************************
Current file path
*******************************************************/
std::string HandleFormat::fetch_pwd(const std::string filename,
                                    const std::string binname)
{
    if(file_dir_found(filename))
    {
        return filename;
    }
    else
    {
        std::string binpath = fetch_f_dir(binname);
        std::string new_file_name  = binpath+'/'+filename;
        return new_file_name;
    }

    return "";
}

/*******************************************
Read ini file
********************************************/
bool HandleFormat::scan_ini_f()
{
	std::ifstream fScanner(input_file_name);
	if (!fScanner)
    {
        return false;
    }

	std::string sentence;
	while (getline(fScanner, sentence))
	{
        line_to_parse.push_back(sentence);
	}
	fScanner.close();

    return true;
}

/***************************************************************************
Go through the lines one by one and modify required words in a given line.
Create new output directory if they don't exist.
**************************************************************************/
bool HandleFormat::modify_format()
{
	bool head_begin_switch = false;
	bool head_close_switch = false;
	std::vector<int> deleted_lines;

	for (int i_line = 0; i_line < line_to_parse.size(); ++i_line)
	{
		size_t location0 = line_to_parse.at(i_line).find("##################");
		if (location0 != std::string::npos) {
			if (!head_begin_switch) {
				head_begin_switch = true;
			}
			else {
				head_begin_switch = false;
			}
		}

		if (head_begin_switch)
		{
            bool subst_ini_with_config = substitute_string(line_to_parse.at(i_line), "ini", "config");
            if(subst_ini_with_config)
            {
                line_to_parse.at(i_line).erase(line_to_parse.at(i_line).begin()+line_to_parse.at(i_line).size()-8,
                                                  line_to_parse.at(i_line).begin()+line_to_parse.at(i_line).size()-5);
            }
            bool subst_eq_with_colon = substitute_string(line_to_parse.at(i_line), "equal sign", "colon");
            if(subst_eq_with_colon)
            {
                line_to_parse.at(i_line).insert(line_to_parse.at(i_line).size()-2, "     ");
            }
			substitute_string(line_to_parse.at(i_line), semicolon_symb, hash_symb);
		}
		else if (head_close_switch)
		{
			substitute_string(line_to_parse.at(i_line), "[", "### ");
			substitute_string(line_to_parse.at(i_line), "]", " ###");

			bool subst_eq_with_colon = substitute_string(line_to_parse.at(i_line), "=", ":");
			if (subst_eq_with_colon)
			{
				std::stringstream splitter(line_to_parse.at(i_line));
				std::string segment;
				std::vector<std::string> split_list;
				while (std::getline(splitter, segment, ' ')) {
					if (segment != "")
						split_list.push_back(segment);
				}

				//path is always second parameter
				for (int i_seg = 0; i_seg < 2; ++i_seg)
				{
					const bool folder_found_switch = folder_found(split_list.at(i_seg));
					if (folder_found_switch)
					{
						std::string file_dir = fetch_f_dir(split_list.at(i_seg));
						std::string para_file_location = fetch_f_dir(input_file_name);

                        std::string new_f_dir;
                        new_f_dir = file_dir;
						bool f_d_found_switch = file_dir_found(new_f_dir);

						if (!f_d_found_switch)
						{
                            char mkdircmd[200];
                            std::string dircmd = "mkdir -p "+new_f_dir;
                            strcpy(mkdircmd,dircmd.c_str());
                            const int dir= system(mkdircmd);
						}
					}
				}
			}

			bool subst_semicol_with_hash = substitute_string(line_to_parse.at(i_line), semicolon_symb, hash_symb);
			if (subst_semicol_with_hash)
			{
				line_to_parse.at(i_line);
				if (i_line + 1 >= line_to_parse.size())
				{
					//Do nothing
				}
				//##//else   // This appends in-line comment after the declaration of a variable, on the same line. Best to disable it, as it causes syntax problems in the CATALOG_COLS: line.
				//##//{
				//##//	line_to_parse.at(i_line + 1) = line_to_parse.at(i_line + 1) + "\t\t\t" + line_to_parse.at(i_line);
				//##//	deleted_lines.push_back(i_line);
				//##//}
			}
		}
		else
		{
			//Both are true
			substitute_string(line_to_parse.at(i_line), semicolon_symb, hash_symb);
			head_close_switch = true;
		}
	}

	//Delete lines using -cur_index
	for (int i_deleted_line = 0; i_deleted_line < deleted_lines.size(); ++i_deleted_line)
	{
		line_to_parse.erase(line_to_parse.begin() + deleted_lines.at(i_deleted_line) - i_deleted_line);
	}

	return true;
}


/***************************************************************************
Replace word in a line
**************************************************************************/
bool HandleFormat::substitute_string(std::string& ln_string, const std::string& inp_string, const std::string new_str)
{
	size_t location = ln_string.find(inp_string);
	while (location == std::string::npos) {
		return false;
	}

	while (location != std::string::npos) {
		ln_string.erase(ln_string.begin() + location, ln_string.begin() + location + inp_string.length());
		ln_string.insert(location, new_str);
		location = ln_string.find(inp_string);
	}

	return true;
}


/**************************************************
Check for folder
***************************************************/
const bool HandleFormat::folder_found(const std::string inp_string)
{
	size_t location = inp_string.find("/");
	if (location != std::string::npos) {
		return true;
	}
	return false;
}


/*********************************************************
Get file path
**********************************************************/
std::string HandleFormat::fetch_f_dir(const std::string inp_string)
{
	std::string directory;
	const size_t last_slash_idx = inp_string.rfind('/');
	if (std::string::npos != last_slash_idx)
	{
		directory = inp_string.substr(0, last_slash_idx);
	}
	return directory;
}


/*********************************************************
Check if file exists
**********************************************************/
const bool HandleFormat::file_dir_found(const std::string inp_string)
{
	struct stat buffer;
	return (stat(inp_string.c_str(), &buffer) == 0);
}


/*********************************************************
Write out config file lines
**********************************************************/
bool HandleFormat::create_config()
{
    //std::string config_dir_and_file;   // This is a global variable

    std::string para_file_location = fetch_f_dir(input_file_name);

    std::set<char> delims{'/', '.'};
    std::string f_name = fetch_f_name(input_file_name, delims);

    std::string config_f_name;
    if(para_file_location != ""){
        config_f_name = para_file_location + '/' + f_name+".cfg";
    }
    else{
        config_f_name = f_name+".cfg";
    }

    config_dir_and_file = config_f_name;   //Store the (path + config file name) combined string as a global variable, so that it can be used widely later

    std::ofstream conf_write(config_f_name);
    if(!conf_write)
    {
        return false;
    }

    for (int i_line = 0; i_line < line_to_parse.size(); ++i_line)
    {
        conf_write << line_to_parse.at(i_line)<<std::endl;
    }

    return true;
}


/************************************************************
Fetch only file name sans the extension
*************************************************************/
std::string HandleFormat::fetch_f_name(const std::string inp_string, const std::set<char> delimiters)
{
  std::vector<std::string> result;

  char const* pointchar = inp_string.c_str();
  char const* start = pointchar;
  for(; *pointchar; ++pointchar)
  {
    if (delimiters.find(*pointchar) != delimiters.end())
    {
      if (start != pointchar)
      {
        std::string str(start, pointchar);
        result.push_back(str);
      }
      else
      {
        result.push_back("");
      }
      start = pointchar + 1;
    }
  }
  result.push_back(start);

  return result.at(result.size()-2);
}
