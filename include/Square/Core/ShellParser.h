#pragma once
#include "Square/Config.h"
#include <variant>
#include <optional>

namespace Square
{
namespace Shell
{
    enum class ErrorType : unsigned char
    {
        none,
        invalid_argument,
        invalid_value
    };

    struct SQUARE_API Error
    {
        std::string what;
        int         id_argument { -1              };
        ErrorType   type        { ErrorType::none };

        Error(Error&&) = default;
        Error(const Error&) = default;
    };

    enum class ValueType : unsigned char
    {
        value_none,   // (just it is present) likes bool
        value_string,
        value_bool,
        value_short,
        value_int,
        value_long,
        value_longlong,
        value_ushort,
        value_uint,
        value_ulonglong,
        value_ulong,
        value_float,
        value_double 
    };
    
    using Value_t = std::variant
    <
        std::string,
        bool,
        short,
        int,
        long,
        long long,
        unsigned short,
        unsigned int,
        unsigned long,
        unsigned long long,
        float, 
        double
    >;

    struct SQUARE_API Command
    {
        std::string            name;
        std::string            short_name;
        std::string            description;
        ValueType             type;
        bool                   mandatory { false };
        std::optional<Value_t> default_value;
    };

    using ParserValue = std::unordered_map
    < 
          std::string
        , Value_t 
    >;

    using ParserReturn = std::tuple
    < 
          Error
        , ParserValue
    >;

    using ParserCommands = std::vector<Command>;

    SQUARE_API ParserReturn parser
    (
        int argc
      , const char* argv[]
      , const ParserCommands& commands
    );

    SQUARE_API std::string help(const ParserCommands& commands);

    SQUARE_API extern const char __path__[];
    SQUARE_API extern const char __filename__[];
    SQUARE_API extern const char __basename__[];
}
}