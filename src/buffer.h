#pragma once
#include <fstream>
/* Swag CRIME lol */
#include <vector>
#include "vector.h"

namespace navmesh {
class Buffer
{
public:
  ~Buffer();

  void Load(std::string_view file);
  void LoadFromMemory(const std::uint8_t *data, std::size_t size);
  void Skip(std::size_t bytes);
  void Clear();

  auto
  Read(void *buffer, std::size_t bytes) -> void
  {
    std::memcpy(buffer, buffer_.data() + position_, bytes);
    position_ += bytes;
  }

  template <typename Type>
  auto
  Read() -> Type
  {
    if (position_ + sizeof(Type) > buffer_.size())
      throw std::out_of_range(
          "Buffer::Read: attempt to read past the end of the buffer");

    Type value = *reinterpret_cast<Type *>(buffer_.data() + position_);
    position_ += sizeof(Type);
    return value;
  }

  template <typename Type>
  auto
  Peek() const -> Type
  {
    if (position_ + sizeof(Type) > buffer_.size())
      throw std::out_of_range(
          "Buffer::Peek: attempt to peek past the end of buffer");

    return *reinterpret_cast<const Type *>(buffer_.data() + position_);
  }

  auto
  bytes_remaining() const -> std::size_t
  {
    return buffer_.size() - position_;
  }

  auto
  position() const -> std::size_t
  {
    return position_;
  }
  void set_position(std::size_t bytes);

private:
  std::size_t               position_ = 0;
  std::vector<std::uint8_t> buffer_;
};
} // namespace navmesh
