#ifndef _I_COMMAND_LINE_ARGUMENTS_H_
#define _I_COMMAND_LINE_ARGUMENTS_H_

// Local
#include "IJobArguments.h"

//! The operation invoked from the command line
enum class ECommandLineOperation
{
    //! The command line arguments are invalid
    INVALID = 0,

    //! Requested to print the version
    VERSION,

    //! Requested to print the help text
    HELP,

    //! Requested to print the interfaces
    LIST_INTERFACES,

    //! Requested to run a replay job
    JOB
};

//! Represents all the arguments passed to the command line
class ICommandLineArguments
{
public:
    virtual ~ICommandLineArguments() = default;

    //! Reads the operation invoked from the CLI
    virtual ECommandLineOperation get_operation() const = 0;

    //! Gets the help text
    //! @throw If operation is not ECommandLineOperation::HELP
    virtual std::string_view get_help() const = 0;

    //! Reads the job arguments passed to the CLI
    //! @throw If operation is not ECommandLineOperation::JOB
    virtual IJobArguments const &get_job_arguments() const = 0;
};

#endif // _I_COMMAND_LINE_ARGUMENTS_H_
