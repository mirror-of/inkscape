#ifndef SIMPLE_IMAGE_H
#define SIMPLE_IMAGE_H

#if _MSC_VER
  #ifdef _M_IX86
    typedef int ssize_t;
  #else
    typedef __int64 ssize_t;
  #endif
#else
  #include <stddef.h>
#endif

// a minimal image representation that allows 2D indexing with [][] and that's completely reusable
template <typename AnyType>
class SimpleImage
{
public:
    SimpleImage()
    {
    }
    SimpleImage(AnyType *b, ssize_t p)
    {
        buffer = b;
        pitch = p;
    }
    AnyType *operator[](ssize_t y)
    {
        return (AnyType *)((uint8_t *)buffer + y * pitch);
    }
    SimpleImage<AnyType> SubImage(ssize_t x, ssize_t y)
    {
        return SimpleImage<AnyType>(&(*this)[y][x], pitch);
    }
    AnyType *buffer;
    ssize_t pitch;
};

template <typename IntType>
IntType RoundDown(IntType a, IntType b)
{
    return (a / b) * b;
}

template <typename IntType>
IntType RoundUp(IntType a, IntType b)
{
    return RoundDown(a + b - 1, b);
}

#ifdef _WIN32
  #define aligned_alloc(a, s) _aligned_malloc(s, a)
  #define aligned_free(x) _aligned_free(x)
#else
  #define aligned_free(x) free(x)
#endif

template <typename AnyType, ssize_t alignment>
class AlignedImage : public SimpleImage<AnyType>
{
public:
  AlignedImage()
  {
    this->buffer = NULL;
  }
  void Resize(int width, int height)
  {
    if (this->buffer != NULL)
      aligned_free(this->buffer);
    this->pitch = RoundUp(ssize_t(width * sizeof(AnyType)), alignment);
    this->buffer = (AnyType *)aligned_alloc(alignment, this->pitch * height);
  }
  ~AlignedImage()
  {
    if (this->buffer != NULL)
      aligned_free(this->buffer);
  }
};

#endif
