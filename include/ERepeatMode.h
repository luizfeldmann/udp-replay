#ifndef _E_REPEAT_MODE_H_
#define _E_REPEAT_MODE_H_

//! Indicates the repetition mode of a job
enum class ERepeatMode
{
    //! No repetition
    NO_REPEAT = 0,

    //! Loop forever
    LOOP,

    //! Repeat X times
    REPEAT_TIMES,
};

#endif // _E_REPEAT_MODE_H_
