#include "Square/Core/StringUtilities.h"

namespace Square
{

bool to_short(const char* arg, short& svalue)
{
    char* end = nullptr;
    long lvalue = std::strtol(arg,&end,10);
    if(arg != end && errno == 0 && lvalue < std::numeric_limits<short>::max() )
    {
        svalue = static_cast<short>(lvalue);
        return true;
    }
    return false;
}

bool to_int(const char* arg, int& ivalue)
{
    char* end = nullptr;
    long lvalue = std::strtol(arg,&end,10);
    if(arg != end && errno == 0 && lvalue < std::numeric_limits<int>::max() )
    {
        ivalue = static_cast<int>(lvalue);
        return true;
    }
    return false;
}

bool to_long(const char* arg, long& lvalue)
{
    char* end = nullptr;
    lvalue = std::strtol(arg,&end,10);
    if(arg != end && errno == 0)
    {
        return true;
    }
    return false;
}

bool to_long_long(const char* arg, long long& llvalue)
{
    char* end = nullptr;
    llvalue = std::strtoull(arg,&end,10);
    if(arg != end && errno == 0)
    {
        return true;
    }
    return false;
}

bool to_unsigned_short(const char* arg, unsigned short& usvalue)
{
    char* end = nullptr;
    unsigned long ulvalue = std::strtoul(arg,&end,10);
    if(arg != end && errno == 0 && ulvalue < std::numeric_limits<unsigned short>::max() )
    {
        usvalue = static_cast<unsigned short>(ulvalue);
        return true;
    }
    return false;
}

bool to_unsigned_int(const char* arg, unsigned int& uivalue)
{
    char* end = nullptr;
    unsigned long ulvalue = std::strtoul(arg,&end,10);
    if(arg != end && errno == 0 && ulvalue < std::numeric_limits<unsigned int>::max() )
    {
        uivalue = static_cast<unsigned int>(ulvalue);
        return true;
    }
    return false;
}

bool to_unsigned_long(const char* arg, unsigned long& ulvalue)
{
    char* end = nullptr;
    ulvalue = std::strtoul(arg,&end,10);
    if(arg != end && errno == 0)
    {
        return true;
    }
    return false;
}

bool to_unsigned_long_long(const char* arg, unsigned long long& ullvalue)
{
    char* end = nullptr;
    ullvalue = std::strtoull(arg,&end,10);
    if(arg != end && errno == 0)
    {
        return true;
    }
    return false;
}

bool to_float(const char* arg, float& fvalue)
{
    char* end = nullptr;
    fvalue = std::strtof(arg,&end);
    if(arg != end && errno == 0)
    {
        return true;
    }
    return false;
}

bool to_double(const char* arg, double& dvalue)
{
    char* end = nullptr;
    dvalue = std::strtod(arg,&end);
    if(arg != end && errno == 0)
    {
        return true;
    }
    return false;
}

bool to_boolean(const char* arg, bool& bvalue)
{
    std::string arg_cpp(arg);
    std::transform(arg_cpp.begin(), arg_cpp.end(), arg_cpp.begin(), ::tolower);
    bvalue =   arg_cpp == "true" 
            || arg_cpp == "yes"
            || arg_cpp == "t"
            || arg_cpp == "y"
            ;
    return bvalue 
    || arg_cpp == "false" 
    || arg_cpp == "no"
    || arg_cpp == "f"
    || arg_cpp == "n"
    ;
}

}