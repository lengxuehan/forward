/** @addtogroup common
 * \ingroup forward
 *  @{ */
/**
* @file errno_exception.h
* @brief Abstract base class for all exceptions related to calls that set the errno variable.
* @details
* @author		wuting.xu
* @date		    2024/10/03
* @par Copyright(c): 	2024. All rights reserved.
*/
#pragma once

#include <cstring>
namespace forward{
namespace common {
namespace exception {
class ErrnoException : public std::runtime_error {
private:
    int32_t error_number_;

public:
    ErrnoException(ErrnoException&& other) = default;
    ErrnoException(const ErrnoException& other) = default;
    ErrnoException& operator=(ErrnoException&& other) = delete;
    ErrnoException& operator=(const ErrnoException& other) = delete;
    virtual ~ErrnoException() noexcept = default;

    /**
     * \brief Gets the error number (errno) recorded in this exception.
     *
     * \returns the errno set in this exception.
     */
    int32_t get_error_number() const {
        return error_number_;
    }

    /**
     * \brief Construct a new ErrnoException and record the error message for the given errno.
     *
     * \param error_number : The C errno that has occurred.
     *
     */
    explicit ErrnoException(const int32_t error_number)
            : std::runtime_error{strerror(error_number)}, error_number_{error_number} {
    }
};

} /* namespace exception */
} /* namespace common */
}/* namespace forward */

/** @}*/    // end of group common