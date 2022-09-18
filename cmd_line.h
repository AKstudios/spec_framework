//==========================================================================================================
// cmd_line.h - Defines a command-line parser
//==========================================================================================================
#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

enum clp_t {CLP_NONE, CLP_REQUIRED, CLP_OPTIONAL};

class CCmdLine
{
public:

    // Prior to calling parse(), declare which switches are valid
    void    declare_switch(std::string name, clp_t swtype);

    // Call this to parse the command line
    bool    parse(int argc, char** argv, bool throw_on_error = false);

    // Call one of these to determine whether a given switch was used
    bool    has_switch(std::string name, std::string *param = NULL);
    bool    has_switch(std::string name, int         *param);
    bool    has_switch(std::string name, double      *param);

    // Returns the count of non-switch command-line arguments
    int     arg_count() {return m_args.size();}

    // Returns the specified non-switch argument
    std::string arg(int index);

    // Returns a vector of every non-switch argument
    std::vector<std::string> args() {return m_args;}

    // After "parse()", this has any error encountered
    std::string error() {return m_error;}

protected:

    // Any errors encountered during parse() are recorded here
    std::string m_error;

    // Maps a switch name to a command-line-parameter-type
    std::map<std::string, clp_t> m_valid_switches;

    // We'll use this structure to track which switches had parameters
    struct param_t {bool exists; std::string value;};

    // Maps a command line switch to its parameter
    std::map<std::string, param_t> m_switches;

    // This contains all of the non-switch arguments
    std::vector<std::string> m_args;
};
