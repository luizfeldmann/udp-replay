#ifndef _C_APP_ERROR_CATEGORY_H_
#define _C_APP_ERROR_CATEGORY_H_

// STD
#include <system_error>

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

#endif // _C_APP_ERROR_CATEGORY_H_