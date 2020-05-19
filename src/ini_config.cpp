#include "ini_config.hpp"

//Converts a given .ini file into a .config file and also creates any non-existent directories mentioned in the .ini file.
void ini_to_config(std::string binname, std::string ini_file_name) {

    //std::string config_dir_and_file;   // This is a global variable

    HandleFormat* fOperate = new HandleFormat();

    ini_file_name = fOperate->fetch_pwd(ini_file_name, binname);

    fOperate->set_ini_name(ini_file_name);

    bool ini_readable = fOperate->scan_ini_f();
    if(!ini_readable){
        std::cout <<"\nERROR: Unable to read the given .ini file!";
        exit(1);
    }

    bool modified = fOperate->modify_format();
    if(!modified){
    std::cout << "\nERROR: Unable to create one or more directory mentioned in the .ini file.";
    exit(1);
    }

    bool status = fOperate->create_config();
    if(!status){
        std::cout << "\nERROR: Unbale to convert config file!";
        exit(1);
    }

    std::cout << "\nCorresponding .cfg file created in the same folder as the .ini file.\n\t"<<std::endl;

    if(fOperate){
        delete fOperate;
        fOperate = nullptr;
    }

}

//Goes through a .config file and creates any non-existent directories mentioned in it.
void config_mkdir(std::string para_file_name) {

    std::ifstream configfile(para_file_name);
    unsigned firstp, lastp;

    if (configfile.is_open()) {                 // Make sure the .config file exists
       std::string line;
       while (std::getline(configfile, line))   // Read each line of the .config file one by one
       {
         std::string word;
         char delim = ' ';

         if (line.find(":")!=std::string::npos) {   // Now choose only those lines that have the ":" character

           //If the line has a comment in it, extract the part of line between the ":" and the "#" characters
           if (line.find(":")!=std::string::npos && line.find("#")!=std::string::npos) {
               firstp = line.find(":");
               lastp = line.find("#");
               std::string strNew = line.substr (firstp,lastp-firstp);
               line = strNew;
           }

           //Now work with this particular line only
           std::istringstream iss(line);

           // For each word of line...
           while (std::getline(iss, word, delim))
           {
               size_t pos = word.find_last_of('/');  //Search for position of the slash character "/" and make sure we choose the last occurance of it in the line

               // Word includes '/'
               if (pos != std::string::npos)  //If a slash character is found, then apply the following
               {
                   std::string dir_name = word.substr(0, pos);
                   // std::cout << dir_name << "\n";

                   char mkdircmd[200];
                   std::string dircmd = "mkdir -p "+dir_name;
                   strcpy(mkdircmd,dircmd.c_str());
                   const int dir= system(mkdircmd);
               }
           }   // Close the loop for going through each word of a line in the .config file

         }

       }   // Close the loop for reading lines of the .config file

         configfile.close();
    }

}
