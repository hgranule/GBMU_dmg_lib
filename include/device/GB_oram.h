#ifndef DEVICE_GB_ORAM_H_
# define DEVICE_GB_ORAM_H_

#include "GB_config.h"

namespace GB::device {

class ORAM {
 public:

    ORAM() : __memory(ORAM_SIZE) {}
    ORAM(const ORAM& other) = default;
    ORAM(ORAM&& other) = default;
    ~ORAM() = default;

    ORAM& operator=(const ORAM& other) = default;
    ORAM& operator=(ORAM&& other) = default;

    inline byte_t read_phys_addr(word_t phys_addr) const;
    inline void write_phys_addr(word_t phys_addr, byte_t value);

    inline const dbuffer_t& get_memory_buffer_ref() const;

 protected:
    dbuffer_t __memory;
};

inline byte_t
ORAM::read_phys_addr(word_t phys_addr) const {
    return __memory[phys_addr];
}

inline void
ORAM::write_phys_addr(word_t phys_addr, byte_t value) {
    __memory[phys_addr] = value;
}

// TODO(dolovnyak) delete if will not be used in oram objects search
inline const dbuffer_t&
ORAM::get_memory_buffer_ref() const {
    return __memory;
}

}   // namespace GB::device

#endif  // DEVICE_GB_ORAM_H_
