//! @file version.h
//! @brief Defines the version information

#ifndef _VERSION_H_
#define _VERSION_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Reads the name of the version
    const char *version_name();

    //! Retuns the description of the project
    const char *version_description();

    //! Reads the version sem-ver
    const char *version_number();

    //! Reads the timestamp of the build
    const char *build_timestamp();

    //! Reads the project's homepage URL
    const char *project_homepage_url();

    //! Reads the license name
    const char *project_license_name();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _VERSION_H_
