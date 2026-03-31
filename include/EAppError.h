#ifndef _E_APP_ERROR_H_
#define _E_APP_ERROR_H_

// STD
#include <system_error>

//! Error codes for the app
enum class EAppError
{
    Unknown = 0,
    GenericArgumentsError,
    MissingArgFile,
    MissingArgInterface,
    ConflictOperations,
    ConflictLimit,
    NoJob,
    UndefinedLimitDuration,
    UndefinedLimitPackets,
    UndefinedSpeedMultiplier,
    UndefinedRepeatCount,
    InvalidRewritePort,
    InvalidMatchPort,
    InvalidPortDelimiter,
    InvalidPortRangeOrder,
    InvalidAddressDelimiter,
    InvalidPrefixLengthUnmatch,
    InvalidIPv6Pair,
};

//! Converts the error code to an error object
std::error_code make_error_code(EAppError);

// Specialization
namespace std
{
    // Register the app error as a recognized error code type
    template <>
    struct is_error_code_enum<EAppError> : true_type
    {
    };
}

#endif // _E_APP_ERROR_H_