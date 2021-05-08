/**
 * @file GB_dbuffer.h
 * @brief Describes dynamic buffer
 */

#ifndef COMMON_GB_DBUFFER_H_
# define COMMON_GB_DBUFFER_H_

# include <cstdint>
# include <utility>
# include <string>
# include <memory>

/**
 * @brief Dynamic allocated buffer with support of move-semantic
 */
class dbuffer_t {
 protected:

    uint8_t*    __data;
    size_t      __len;

 protected:

    inline static
    uint8_t* __allocate(size_t n) {
        return new uint8_t[n];
    }

    inline static void
    __deallocate(uint8_t *data) {
        delete[] data;
    }

    inline void __free() {
        if (__data)
            __deallocate(__data);
        __data = nullptr;
    }

 public:

    ~dbuffer_t() {
        __free();
    }

    /**
     * @brief Create and allocate dynamic buffer with some size
     * @param[in] size size (in bytes) to allocate for buffer data
     */
    explicit
    dbuffer_t(size_t size = 0ul)
    : __data(size != 0 ? __allocate(size) : nullptr)
    , __len(size) {
    }

    /**
     * @brief Create and allocate dynamic buffer with some content
     * @param[in] ext_data address from which content will be copied
     * @param[in] length size in bytes, to copy from ext_data
     */
    explicit
    dbuffer_t(void* ext_data, size_t lenght)
    : __data(__allocate(lenght))
    , __len(lenght) {
        memcpy(__data, ext_data, __len);
    }

    /**
     * @brief Create and allocate dynamic buffer with some content
     * @param[in] str string from which content will be copied
     */
    explicit
    dbuffer_t(const std::string& str)
    : __data(__allocate(str.length()))
    , __len(str.length()) {
        memcpy(__data, str.data(), __len);
    }

    dbuffer_t(const dbuffer_t& source)
    : __data(__allocate(source.__len))
    , __len(source.__len) {
        memcpy(__data, source.__data, __len);
    }

    dbuffer_t(dbuffer_t&& source) noexcept
    : __data(std::move(source.__data))
    , __len(std::move(source.__len)) {
        source.__data = nullptr;
        source.__len = 0;
    }

    dbuffer_t& operator=(const dbuffer_t& source) {
        if (this != &source) {
            __free();
            __data = __allocate(source.__len);
            __len = source.__len;
            memcpy(__data, source.__data, __len);
        }
        return *this;
    }

    dbuffer_t& operator=(dbuffer_t&& source) noexcept {
        if (this != &source) {
            __free();
            __data = std::move(source.__data);
            __len = std::move(source.__len);

            source.__data = nullptr;
            source.__len = 0;
        }
        return *this;
    }


    /**
     * @brief get address of buffers data
     */
    inline uint8_t* get_data_addr() {
        return __data;
    }

    /**
     * @brief get address of buffers data
     */
    inline uint8_t* get_data_addr() const {
        return __data;
    }

    /**
     * @brief get access to byte at some offset
     */
    inline uint8_t& operator[](size_t offset) {
        return __data[offset];
    }

    /**
     * @brief get access to byte at some offset
     */
    inline const uint8_t& operator[](size_t offset) const {
        return __data[offset];
    }

};

#endif  // COMMON_GB_DBUFFER_H_
