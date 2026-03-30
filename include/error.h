#ifndef _ERROR_H_
#define _ERROR_H_

// STD
#include <system_error>

//! Error codes for the app
enum class EAppError
{
    Unknown = 0,
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

//! Category for the errors in this app
class CAppErrorCategory
    : public std::error_category
{
public:
    //! Reads the name of the category
    const char *name() const noexcept override;

    //! Gets the error message from the error code
    std::string message(int ev) const override;
};

//! Gets the singleton of the app error category
const std::error_category &app_error_category();

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

#endif // _ERROR_H_