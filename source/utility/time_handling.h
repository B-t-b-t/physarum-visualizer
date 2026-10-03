#ifndef TIME_HANDLING_H
#define TIME_HANDLING_H

#include <chrono>

struct TimeSlot {
    std::chrono::system_clock::time_point start{};
    std::chrono::system_clock::time_point end{};

    TimeSlot() = default;   //initializes both start and end to UNIX time beginning (1970-01-01...)

    TimeSlot(std::chrono::system_clock::time_point startIn, std::chrono::system_clock::time_point endIn)
        : start{startIn}, end{endIn} 
    {

    }

    /**
     * @brief Checks if a given time point falls within the [inclusive] range of the time slot.
     * 
     * @param time The time point to check.
     * @return True if the time is within the start and end time, False otherwise.
     */
    bool isDuring(const std::chrono::system_clock::time_point& time) const {
        return time >= start && time <= end;
    }
    
    /**
     * @brief Checks if start time is before end time.
     * 
     * @return True if start is before end, False otherwise.
     * @note A default constructed TimeSlot is not considered valid.
     */
    bool isValid() const {
        return start < end; //omitting equality ensures, that default constructed TimeSlots are not considered valid
    }
    
    //for validity checks in Ifs and other conditional statements
    explicit operator bool() const {
        return isValid();
    }
};

#endif // TIME_HANDLING_H