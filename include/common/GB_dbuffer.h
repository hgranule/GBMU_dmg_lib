/**
 * @file GB_dbuffer.h
 * @brief Describes dynamic buffer
 */

#ifndef GB_DBUFFER_H_
# define GB_DBUFFER_H_

# include <cstdint>
# include <memory>

/**
 * @brief Dynamic allocated buffer with support of move-semantic
 */
template < typename _Allocator = std::allocator<uint8_t> >
class DBuffer {
public:

    using ValueType = uint8_t;
    using Pointer = ValueType *;
    using ConstPointer = const ValueType *;
    using Reference = ValueType &;
    using ConstReference = const ValueType &;

private:

    _Allocator  __allocator;
    Pointer     __data;
    size_t      __len;

private:

    inline void __Free() {
        if (__data)
            __allocator.deallocate(__data, __len);
        __data = nullptr;
    }

public:

    ~DBuffer() {
        __Free();
    }

    /**
     * @brief Create and allocate dynamic buffer with some size
     * @param[in] size size (in bytes) to allocate for buffer data
     */
    DBuffer(size_t size = 0ul)
    : __allocator(_Allocator())
    , __data(size != 0 ? __allocator.allocate(size) : nullptr)
    , __len(size)
    {
    }

    /**
     * @brief Create and allocate dynamic buffer with some content
     * @param[in] extData address from which content will be copied
     * @param[in] length size in bytes, to copy from extData
     */
    DBuffer(void* extData, size_t lenght)
    : __allocator(_Allocator())
    , __data(__allocator.allocate(lenght))
    , __len(lenght)
    {
        memcpy(__data, extData, __len);
    }

    /**
     * @brief Create and allocate dynamic buffer with some content
     * @param[in] str string from which content will be copied
     */
    DBuffer(const std::string& str)
    : __allocator(_Allocator())
    , __data(__allocator.allocate(str.length()))
    , __len(str.length())
    {
        memcpy(__data, str.data(), __len);
    }

    DBuffer(const DBuffer& source)
    : __allocator(source.__allocator)
    , __data(__allocator.allocate(source.__len))
    , __len(source.__len)
    {
        memcpy(__data, source.__data, __len);
    }

    DBuffer(DBuffer&& source)
    : __allocator(std::move(source.__allocator))
    , __data(std::move(source.__data))
    , __len(std::move(source.__len))
    {
        source.__data = nullptr;
        source.__len = 0;
    }

    DBuffer& operator=(const DBuffer& source);
    DBuffer& operator=(DBuffer&& source);

    /**
     * @brief get address of buffers data
     */
    inline Pointer Data() {
        return __data;
    }

    /**
     * @brief get address of buffers data
     */
    inline ConstPointer Data() const {
        return __data;
    }

    /**
     * @brief get access to byte at some offset
     */
    inline Reference operator[](size_t offset) {
        return __data[offset];
    }

    /**
     * @brief get access to byte at some offset
     */
    inline ConstReference operator[](size_t offset) const {
        return __data[offset];
    }

};

template <typename _Allocator>
auto DBuffer<_Allocator>::operator=(const DBuffer& source) -> DBuffer&
{
    if (this != &source) {
        __Free();
        __allocator = source.__allocator;
        __data = __allocator.allocate(source.__len);
        __len = source.__len;
        memcpy(__data, source.__data, __len);
    }
    return *this;
}

template <typename _Allocator>
auto DBuffer<_Allocator>::operator=(DBuffer&& source) -> DBuffer&
{
    if (this != &source) {
        __Free();
        __allocator = std::move(source.__allocator);
        __data = std::move(source.__data);
        __len = std::move(source.__len);

        source.__data = nullptr;
        source.__len = 0;
    }
    return *this;
}

#endif
