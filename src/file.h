#pragma once
#include "buffer.h"
#include "generation_params.h"
#include "ladder.h"
#include "area.h"

namespace navmesh {
class File
{
public:
  File() = default;
  explicit File(std::string_view file);
  File(const std::uint8_t *data, std::size_t size);
  ~File();
  void Load(std::string_view file);
  void LoadFromMemory(const std::uint8_t *data, std::size_t size);

  auto
  area_by_id(std::uint32_t id) -> Area &
  {
    for (auto &area : areas_) {
      if (area.id() == id)
        return area;
    }
    throw std::runtime_error("File::area_by_id: failed to find area");
  }

  auto
  area_by_position(Vector3_t position) -> Area &
  {
    for (auto &area : areas_) {
      if (area.IsWithin(position))
        return area;
    }
    throw std::runtime_error("File::area_by_position: failed to find area");
  }

  auto
  version() const -> std::uint32_t
  {
    return version_;
  }
  auto
  polygons() const -> const std::vector<std::vector<Vector3_t>> &
  {
    return polygons_;
  }
  auto
  area_count() const -> std::uint32_t
  {
    return static_cast<std::uint32_t>(areas_.size());
  }
  auto
  is_analyzed() const -> bool
  {
    return is_analyzed_;
  }
  auto
  ladders() const -> std::vector<Ladder>
  {
    return ladders_;
  }

  auto
  hull_params_for_area(const Area &area) const -> const GenerationHullParams &
  {
    auto hull_index = area.hull_index();
    if (hull_index < generation_params_.hull_params.size())
      return generation_params_.hull_params[hull_index];

    if (!generation_params_.hull_params.empty())
      return generation_params_.hull_params[0];

    static GenerationHullParams empty;
    return empty;
  }

  std::string ReadNullTerminatedString();

private:
  void Parse();
  void Unload();
  void ReadPolygons();
  void ReadLadders();
  void ReadGenerationParams();
  void SkipKV3();

  Buffer buffer_;

  static constexpr std::uint32_t kMagic = 0xFEEDFACE;
  std::uint32_t                  version_ = 0;
  std::uint32_t                  sub_version_ = 0;
  bool                           is_analyzed_ = false;

  std::vector<std::vector<Vector3_t>> polygons_;
  std::vector<Area>                   areas_;
  std::vector<Ladder>                 ladders_;

  GenerationParams         generation_params_;
  std::vector<std::string> places_;
};
} // namespace navmesh
