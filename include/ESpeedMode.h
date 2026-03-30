#ifndef _E_SPEED_MODE_H_
#define _E_SPEED_MODE_H_

//! Defines the speed throttling mode
enum class ESpeedMode
{
    //! No speed throttling
    NO_SPEED,

    //! The job adjusts the speed based on a multiplier
    SPEED_MULTIPLIER,
};

#endif
