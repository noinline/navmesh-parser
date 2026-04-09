#include <iterator>
#include "buffer.h"

namespace navmesh {
Buffer::~Buffer() { Clear(); }

auto
Buffer::Load(std::string_view file) -> void
{
  Clear();

  std::ifstream nav_file(file.data(), std::ifstream::binary);
  if (!nav_file.is_open())
    throw std::runtime_error("Buffer::Load: couldn't open .nav file, are you "
                             "trying to feed me some empty data?");

  nav_file.unsetf(std::istream::skipws);

  nav_file.seekg(0, std::istream::end);
  auto size = nav_file.tellg();
  nav_file.seekg(0, std::istream::beg);

  buffer_.resize(static_cast<std::size_t>(size));
  nav_file.read(reinterpret_cast<char *>(buffer_.data()), size);
}

auto
Buffer::LoadFromMemory(const std::uint8_t *data, std::size_t size) -> void
{
  Clear();

  if (!data || size == 0)
    throw std::runtime_error("Buffer::LoadFromMemory: invalid data or size");

  buffer_.assign(data, data + size);
}

auto
Buffer::Skip(std::size_t bytes) -> void
{
  if (position_ + bytes > buffer_.size())
    throw std::out_of_range("Buffer::Skip: attempt to skip past end of buffer");

  position_ += bytes;
}

auto
Buffer::set_position(std::size_t bytes) -> void
{
  if (bytes > buffer_.size())
    throw std::out_of_range("Buffer::set_position: position out of range");

  position_ = bytes;
}

auto
Buffer::Clear() -> void
{
  buffer_.clear();
  position_ = 0;
}
} // namespace navmesh
