#include "file.h"
#include "kv3.h"

namespace navmesh {
File::File(std::string_view file) { Load(file); }

File::File(const std::uint8_t *data, std::size_t size) { LoadFromMemory(data, size); }

File::~File() { Unload(); }

auto
File::LoadFromMemory(const std::uint8_t *data, std::size_t size) -> void
{
  buffer_.LoadFromMemory(data, size);
  Parse();
}

auto
File::Load(std::string_view file) -> void
{
  buffer_.Load(file);
  Parse();
}

auto
File::Parse() -> void
{
  auto magic = buffer_.Read<std::uint32_t>();
  if (magic != kMagic)
    throw std::runtime_error(
        "File::Parse: magic mismatch, what are you trying to feed me? -_-");

  version_ = buffer_.Read<std::uint32_t>();
  if (version_ < 31 || version_ > 36)
    throw std::runtime_error("File::Parse: unsupported version");

  sub_version_ = buffer_.Read<std::uint32_t>();

  auto flags = buffer_.Read<std::uint32_t>();
  is_analyzed_ = (flags & 0x00000001) != 0;

  if (version_ >= 36)
    SkipKV3();

  if (version_ >= 31)
    ReadPolygons();

  [[maybe_unused]] std::uint32_t unk2 = 0;
  if (version_ >= 32)
    unk2 = buffer_.Read<std::uint32_t>();

  if (version_ >= 35) {
    auto unk_count1 = buffer_.Read<std::uint32_t>();
    for (std::uint32_t i = 0; i < unk_count1; ++i) {
      auto str = ReadNullTerminatedString();
      buffer_.Skip(48);
    }
  }

  if (version_ >= 36)
    SkipKV3();

  auto area_count = buffer_.Read<std::uint32_t>();
  areas_.reserve(area_count);

  for (std::uint32_t i = 0; i < area_count; ++i) {
    Area area(buffer_, *this);
    areas_.push_back(std::move(area));
  }

  ReadLadders();

  auto unk_count2 = buffer_.Read<std::uint32_t>();
  buffer_.Skip(unk_count2 * 18 * sizeof(float));

  ReadGenerationParams();

  if (sub_version_ > 0)
    SkipKV3();
}

auto
File::Unload() -> void
{
  buffer_.Clear();

  version_ = 0;
  sub_version_ = 0;
  is_analyzed_ = false;

  polygons_.clear();
  areas_.clear();
  ladders_.clear();

  generation_params_ = GenerationParams();
  places_.clear();
}

auto
File::ReadPolygons() -> void
{
  auto                   corner_count = buffer_.Read<std::uint32_t>();
  std::vector<Vector3_t> corners(corner_count);

  for (std::uint32_t i = 0; i < corner_count; ++i) {
    corners[i].at(0) = buffer_.Read<float>();
    corners[i].at(1) = buffer_.Read<float>();
    corners[i].at(2) = buffer_.Read<float>();
  }

  auto polygon_count = buffer_.Read<std::uint32_t>();
  polygons_.resize(polygon_count);

  for (std::uint32_t i = 0; i < polygon_count; ++i) {
    auto corner_count_in_poly = buffer_.Read<std::uint8_t>();
    polygons_[i].resize(corner_count_in_poly);

    for (std::uint8_t j = 0; j < corner_count_in_poly; ++j) {
      auto corner_index = buffer_.Read<std::uint32_t>();
      if (corner_index < corners.size())
        polygons_[i][j] = corners[corner_index];
      else
        throw std::runtime_error(
            "File::ReadPolygons: corner index out of range");
    }
    [[maybe_unused]] std::uint32_t unk = 0;
    if (version_ >= 35)
      unk = buffer_.Read<std::uint32_t>();
  }
}

auto
File::ReadLadders() -> void
{
  auto ladder_count = buffer_.Read<std::uint32_t>();
  ladders_.resize(ladder_count);

  for (std::uint32_t i = 0; i < ladder_count; ++i) {
    Ladder ladder(buffer_, *this);
    ladders_[i] = std::move(ladder);
  }
}

auto
File::ReadGenerationParams() -> void
{
  generation_params_.Load(buffer_, *this);
}

auto
File::ReadNullTerminatedString() -> std::string
{
  std::string result;
  char        c;

  while (buffer_.bytes_remaining() > 0) {
    c = buffer_.Read<char>();
    if (c == 0)
      break;

    result.push_back(c);
  }

  return result;
}

auto
File::SkipKV3() -> void
{
  try {
    if (KV3::IsBinary(buffer_))
      KV3::Skip(buffer_);
  } catch (...) {
    throw std::runtime_error("File::SkipKV3: explosion happened \\||/");
  }
}
} // namespace navmesh
