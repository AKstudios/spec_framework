//==========================================================================================================
// cmd_line.cpp - Implements a command-line parser
//==========================================================================================================
#include <stdlib.h>
#include "cmd_line.h"
using namespace std;

static double s_to_d(const string& s)  {return strtod(s.c_str(), NULL   );}
static int    s_to_i(const string& s)  {return strtol(s.c_str(), NULL, 0);}


//==========================================================================================================
// declare_switch() - Gets called prior to parse() in order to declare with command-line switches are valid
//==========================================================================================================
void CCmdLine::declare_switch(string name, clp_t swtype)
{
    // Make sure that switch names begin with a dash
    if (name[0] != '-') name = '-' + name;
    
    // And add this switch to the list of valid command-line switches
    m_valid_switches[name] = swtype;
}
//==========================================================================================================


//==========================================================================================================
// parse() - Parses the command line
//
// Passed: argc           = argc from main()
//         argv           = argv from main()
//         throw_on_error = If this is true, errors will be reported via throwing a runtime exception
//                          instead of by returning true/false
//
// Returns: success status
//          If return value is false, error string can be retrieved by calling "error()"
//==========================================================================================================
bool CCmdLine::parse(int argc, char** argv, bool throw_on_error)
{
    int i = 0;
    param_t param;
    map<string, clp_t>::iterator it;

    // Clear any existing command line data we have
    m_switches.clear();
    m_args.clear();

    // Loop through every token on the command line...
    while (argv[++i])
    {
        // Fetch this argument from the command line
        const string& token = argv[i];
        
        // If this token isn't a switch, just append it to the list of non-switch arguments
        if (token[0] != '-')
        {
            m_args.push_back(token);
            continue;
        }

        // Is this switch in our list of valid switches?
        it = m_valid_switches.find(token);

        // If it's not a valid command-line switch for this program, complain to the user
        if (it == m_valid_switches.end())
        {
            m_error = "'" + token + "' is not a valid switch";
            if (throw_on_error) throw runtime_error(m_error);
            return false;
        }

        // Is the parameter for this switch optional, required, or none?
        clp_t swtype = it->second;

        // Presume for the moment that the user didn't supply a switch parameter
        param.value = "";

        // Find out if the user supplied a switch parameter
        param.exists = (argv[i+1] && argv[i+1][0] != '-');

        // If this switch doesn't have a parameter and it was supposed to, complain to the user
        if (!param.exists && swtype == CLP_REQUIRED)
        {
            m_error = "switch '" + token + "' requires a parameter";
            if (throw_on_error) throw runtime_error(m_error);
            return false;
        }

        // If there is a parameter, and this switch can accept one, fetch it
        if (param.exists && (swtype == CLP_REQUIRED || swtype == CLP_OPTIONAL))
        {
            param.value = argv[++i];
        }

        // Store the switch and its parameter (if any)
        m_switches[token] = param;
    }

    // No error was encountered
    m_error.clear();

    // Tell the caller that all is well
    return true;
}
//==========================================================================================================



//==========================================================================================================
// has_switch() - Returns 'true' if the specified switch exists on the command line
//
// Note: For switches with an optional parameter, *param will only be filled in if the user
//       supplied a switch parameter on the command line
//==========================================================================================================
bool CCmdLine::has_switch(string name, string *param)
{
    map<string, param_t>::iterator it;

    // Make sure that switch names begin with a dash
    if (name[0] != '-') name = '-' + name;

    // Did the user use the specified switch on the command line?
    it = m_switches.find(name);

    // If the user didn't specify this switch on the command line, tell the caller
    if (it == m_switches.end()) return false;

    // Otherwise, the user *did* use this switch.  If a param exists, hand it to the caller.
    if (param && it->second.exists) *param = it->second.value;

    // And tell the caller that the specified switch was used on the command line
    return true;
}
//==========================================================================================================



//==========================================================================================================
// has_switch() - Returns 'true' if the specified switch exists on the command line
//==========================================================================================================
bool CCmdLine::has_switch(string name, int* param)
{
    map<string, param_t>::iterator it;

    // Make sure that switch names begin with a dash
    if (name[0] != '-') name = '-' + name;

    // Did the user use the specified switch on the command line?
    it = m_switches.find(name);

    // If the user didn't specify this switch on the command line, tell the caller
    if (it == m_switches.end()) return false;

    // Otherwise, the user *did* use this switch.  If a param exists, hand it to the caller.
    if (it->second.exists) *param = s_to_i(it->second.value);

    // And tell the caller that the specified switch was used on the command line
    return true;
}
//==========================================================================================================


//==========================================================================================================
// has_switch() - Returns 'true' if the specified switch exists on the command line
//==========================================================================================================
bool CCmdLine::has_switch(string name, double* param)
{
    map<string, param_t>::iterator it;

    // Make sure that switch names begin with a dash
    if (name[0] != '-') name = '-' + name;

    // Did the user use the specified switch on the command line?
    it = m_switches.find(name);

    // If the user didn't specify this switch on the command line, tell the caller
    if (it == m_switches.end()) return false;

    // Otherwise, the user *did* use this switch.  If a param exists, hand it to the caller.
    if (it->second.exists) *param = s_to_d(it->second.value);

    // And tell the caller that the specified switch was used on the command line
    return true;
}
//==========================================================================================================


//==========================================================================================================
// arg() - Fetches the non-switch argument with the specified index
//==========================================================================================================
string CCmdLine::arg(int index)
{
    if (index >=0 && index < m_args.size()) return m_args[index];
    return "";
}
//==========================================================================================================

