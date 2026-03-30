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

const char *build_timestamp()
{
    static const char *szVersionBuildTimestamp = "${BUILD_TIME}";
    return szVersionBuildTimestamp;
}

const char *project_homepage_url()
{
    static const char *szHomepageURL = "${CMAKE_PROJECT_HOMEPAGE_URL}";
    return szHomepageURL;
}

const char *project_license_name()
{
    static const char *szLicenseName = "${CMAKE_PROJECT_SPDX_LICENSE}";
    return szLicenseName;
}
