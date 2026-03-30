// Local project
#include "version.h"

const char *version_name()
{
    static const char *szVersionName = "${CMAKE_PROJECT_NAME}";
    return szVersionName;
}

const char *version_description()
{
    static const char *szVersionDesc = "${CMAKE_PROJECT_DESCRIPTION}";
    return szVersionDesc;
}

const char *version_number()
{
    static const char *szVersionNumber = "${CMAKE_PROJECT_VERSION}";
    return szVersionNumber;
}
