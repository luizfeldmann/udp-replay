// Local
#include "error.h"
#include "version.h"

const std::error_category &app_error_category()
{
    static const CAppErrorCategory instance;
    return instance;
}

std::error_code make_error_code(EAppError e)
{
    return {static_cast<int>(e), app_error_category()};
}

const char *CAppErrorCategory::name() const noexcept
{
    return version_name();
}

std::string CAppErrorCategory::message(int ev) const
{
    switch (static_cast<EAppError>(ev))
    {
    default:
    case EAppError::Unknown:
        return "unknown error";

    case EAppError::MissingArgFile:
        return "missing required input file path";

    case EAppError::MissingArgInterface:
        return "missing required interface name";

    case EAppError::ConflictOperations:
        return "conflicting options";

    case EAppError::ConflictLimit:
        return "conflicting options for duration limit and number of packets limit";

    case EAppError::NoJob:
        return "no job arguments specified";

    case EAppError::UndefinedLimitDuration:
        return "undefined duration limit";

    case EAppError::UndefinedLimitPackets:
        return "undefined packets limit";

    case EAppError::UndefinedSpeedMultiplier:
        return "undefined speed multiplier";

    case EAppError::UndefinedRepeatCount:
        return "undefined repeat count";

    case EAppError::InvalidRewritePort:
        return "invalid rewritten port";

    case EAppError::InvalidMatchPort:
        return "invalid matched port";

    case EAppError::InvalidPortDelimiter:
        return "expected colon (':') delimiter between match and rewrite port numbers";

    case EAppError::InvalidPortRangeOrder:
        return "ports range must have an end port greater than the start port";

    case EAppError::InvalidAddressDelimiter:
        return "expected colon (':') delimiter in address rewrite pair";

    case EAppError::InvalidPrefixLengthUnmatch:
        return "the networks in the pair have different prefix lengths";

    case EAppError::InvalidIPv6Pair:
        return "expected IPv6 pair in format '[abcd:.../xy]:[abcd:.../xy]'";
    }
}