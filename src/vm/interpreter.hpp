#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#include "codec.hpp"

namespace SCM::Vm
{

constexpr std::size_t DataMemorySize = 1 << 16;

struct State
{
    std::uint32_t accumulator{};
    std::vector<std::uint32_t> data = std::vector<std::uint32_t>(DataMemorySize);
};

struct Range
{
    std::uint32_t first;
    std::uint32_t last;
};

std::expected<void, std::string> execute(State& state, const Codec::Instruction& instruction);
std::expected<State, std::string> run(std::span<const std::byte> code);

std::expected<std::string, std::string> toXml(const State& state, Range range);

std::expected<std::vector<std::byte>, std::string> readBinary(const std::filesystem::path& path);
std::expected<void, std::string> writeDump(const std::filesystem::path& path,
                                           const State& state,
                                           Range range);

} // namespace SCM::Vm
