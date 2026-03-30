#ifndef _PARSE_CLI_H_
#define _PARSE_CLI_H_

// Local
#include "ICommandLineArguments.h"

// STD
#include <memory>

//! Parses the command line arguments and returns an accessor interface to read them
std::unique_ptr<ICommandLineArguments> parse_cli(int argc, const char **argv);

#endif // _PARSE_CLI_H_
