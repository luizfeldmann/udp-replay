// Local
#include "EAppError.h"
#include "CAppErrorCategory.h"

std::error_code make_error_code(EAppError e)
{
    return {static_cast<int>(e), app_error_category()};
}
