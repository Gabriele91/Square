#include "Square/Core/ShellParser.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Core/Filesystem.h"

namespace Square
{

namespace Shell
{
    const char __path__[] = "__path__";
    const char __filename__[] = "__filename__";
    const char __basename__[] = "__basename__";

    ParserReturn parser
    (
        int argc
      , const char* argv[]
      , const ParserCommands& commands
    )
    {
        enum class ParserState : unsigned char
        {
            arg,
            value
        };

        ParserState state
        {
            ParserState::arg
        };

        // Values
        ParserValue values;
        // Last command
        ParserCommands::const_iterator last_command;
        // App name
        if (1 < argc)
        {
            values[__path__] = argv[0];
            values[__filename__] = Filesystem::get_filename(argv[0]);
            values[__basename__] = Filesystem::get_basename(argv[0]);
        }
        // Parsing
        for(int c = 1; c < argc; ++c)
        {
            // Alias
            const char* arg = argv[c];
            // Test
            switch (state)
            {
            case  ParserState::arg:
            {
                size_t arg_size = std::strlen(arg);
                // To short
                if (arg_size < 2) 
                {    
                    return std::make_tuple
                    (
                        Error
                        {
                              std::move(std::string("Invalid argument size"))
                            , c
                            , ErrorType::invalid_argument
                        }
                        , std::move(values)
                    );
                }
                // Bad case
                if (arg[0] != '-') 
                {    
                    return std::make_tuple
                    (
                        Error
                        {
                              std::move(std::string("Invalid argument name"))
                            , c
                            , ErrorType::invalid_argument
                        }
                        , std::move(values)
                    );
                }
                // -value or --value cases
                else
                { 
                    // Looks for --name or -short_name
                    last_command = arg[1] == '-' 
                    ? std::find_if(commands.cbegin(), commands.cend(), [arg](const Command& cmd) -> bool
                    {
                        return cmd.name == &arg[2];
                    }) 
                    : std::find_if(commands.cbegin(), commands.cend(), [arg](const Command& cmd) -> bool
                    {
                        return cmd.short_name == &arg[1];
                    });
                    // Test it
                    if(last_command == commands.end())
                    {    
                        return std::make_tuple
                        (
                            Error
                            {
                                  std::move(std::string("Invalid argument name (" + std::string(arg) + ")"))
                                , c
                                , ErrorType::invalid_argument
                            }
                            , std::move(values)
                        );
                    }
                    // If it is none, just check if it is present
                    if(last_command->type == ValueType::value_none)
                    {
                        values[last_command->name] = true;
                    }
                    else
                    {
                        // Ok read the argument
                        state = ParserState::value;
                    }
                }
            }
            break;
            case  ParserState::value:
            {
                // Args
                Value_t current_value;
                bool ok = false;

                // Parser
                switch (last_command->type)
                {
                    default:
                    case ValueType::value_string:   { auto [rok,rval] = to_<std::string>(arg);         ok = rok;  current_value = rval; } break;
                    case ValueType::value_bool:     { auto [rok,rval] = to_<bool>(arg);                ok = rok;  current_value = rval; } break;
                    case ValueType::value_short:    { auto [rok,rval] = to_<short>(arg);               ok = rok;  current_value = rval; } break;
                    case ValueType::value_int:      { auto [rok,rval] = to_<int>(arg);                 ok = rok;  current_value = rval; } break;
                    case ValueType::value_long:     { auto [rok,rval] = to_<long>(arg);                ok = rok;  current_value = rval; } break;
                    case ValueType::value_longlong: { auto [rok,rval] = to_<long long>(arg);           ok = rok;  current_value = rval; } break;
                    case ValueType::value_ushort:   { auto [rok,rval] = to_<unsigned short>(arg);      ok = rok;  current_value = rval; } break;
                    case ValueType::value_uint:     { auto [rok,rval] = to_<unsigned int>(arg);        ok = rok;  current_value = rval; } break;
                    case ValueType::value_ulong:    { auto [rok,rval] = to_<unsigned long>(arg);       ok = rok;  current_value = rval; } break;
                    case ValueType::value_ulonglong:{ auto [rok,rval] = to_<unsigned long long>(arg);  ok = rok;  current_value = rval; } break;
                    case ValueType::value_float:    { auto [rok,rval] = to_<float>(arg);               ok = rok;  current_value = rval; } break;
                    case ValueType::value_double:   { auto [rok,rval] = to_<double>(arg);              ok = rok;  current_value = rval; } break;
                }

                // Test error
                if(!ok)
                {
                    return std::make_tuple
                    (
                        Error
                        {
                              std::move(std::string("Invalid value"))
                            , c
                            , ErrorType::invalid_value
                        }
                        , std::move(values)
                    );
                }

                // Add value
                values[last_command->name] = std::move(current_value);

                // Ok
                state = ParserState::arg;
            }
            break;
            // Wrong
            default:
                return std::make_tuple
                (
                      Error
                      {
                            std::move(std::string("Invalid input"))
                          , c
                          , ErrorType::invalid_argument
                      }
                    , std::move(values)
                );
            }
        }
       
        // Test state
        if(state != ParserState::arg)
        {
            return std::make_tuple
            (
                  Error
                  {
                      std::move(std::string("Miss a value"))
                    , argc
                    , ErrorType::invalid_value
                  }
                , std::move(values)
            );
        }
        
        // Fill default 
        for(auto& cmd : commands)
        {
            if(cmd.default_value.has_value() && values.find(cmd.name) == values.end())
            {
                values[cmd.name] = *cmd.default_value;
            }
        }

        // Test mandatory
        for(auto& cmd : commands)
        {
            if(cmd.mandatory && values.find(cmd.name) == values.end())
            {
                return std::make_tuple
                (
                      Error
                      {
                            std::move(std::string("Mandatory argument (" + cmd.name + ") is missed"))
                          , argc
                          , ErrorType::invalid_argument
                      }
                    , std::move(values)
                );
            }
        }

        // Ok
        return std::make_tuple(Error{}, std::move(values));
    }
    
    std::string help(const ParserCommands& commands)
    {
        // Get max line size
        size_t max_size_of_name_plus_short_name = 0;
        size_t max_size_of_description = 0;

        // Compute
        for(const auto& cmd : commands)
        {
            max_size_of_name_plus_short_name = (std::max<size_t>)(cmd.name.size() + cmd.short_name.size(), max_size_of_name_plus_short_name);
            max_size_of_description = (std::max<size_t>)(cmd.description.size(), max_size_of_description);
        }

        // Buffer
        std::stringstream ss_help;

        // Print all commands
        for(const auto& cmd : commands)
        {
            ss_help 
            << "--" << cmd.name << ", -" << cmd.short_name 
            << std::string(max_size_of_name_plus_short_name - (cmd.name.size() + cmd.short_name.size()) + 4, ' ')
            << cmd.description
            << std::string(max_size_of_description - cmd.description.size() + 4, ' ')
            << std::endl;
        }
        
        return ss_help.str();
    }
}
}