#ifndef _E_LIMIT_MODE_H_
#define _E_LIMIT_MODE_H_

//! Defines the mode of packet limiting
enum class ELimitMode
{
    //! Job is not limited
    NO_LIMIT = 0,

    //! Limited by maximum number of packets
    LIMIT_MAX_PACKETS,

    //! Limited by maximum time
    LIMIT_MAX_TIME,
};

#endif // _E_LIMIT_MODE_H_