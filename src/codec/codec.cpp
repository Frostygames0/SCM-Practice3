#include "codec.hpp"

#include <optional>

namespace SCM::Codec
{

namespace
{
constexpr std::uint64_t ByteMask = 0xFF;
constexpr unsigned BitsPerByte = 8;
constexpr std::uint64_t OpcodeMask = (1U << OpcodeBits) - 1;
} // namespace

EncodedBytes encode(const Instruction& instruction)
{
    std::uint64_t word = static_cast<std::uint64_t>(instruction.opcode) |
                         (static_cast<std::uint64_t>(instruction.arg) << OpcodeBits);

    EncodedBytes out;

    for (std::size_t i{}; i < out.size(); ++i)
    {
        out[i] = static_cast<std::byte>((word >> (BitsPerByte * i)) & ByteMask);
    }

    return out;
}

std::optional<Instruction> decode(const EncodedBytes& bytes)
{
    std::uint64_t word{};

    for (std::size_t i{}; i < bytes.size(); ++i)
    {
        word |= static_cast<std::uint64_t>(bytes[i]) << (BitsPerByte * i);
    }

    const auto* info = findInfo(static_cast<Opcode>(word & OpcodeMask));
    if (info == nullptr)
    {
        return std::nullopt;
    }

    const std::uint64_t args = word >> OpcodeBits;
    if ((args >> info->argBits) != 0)
    {
        return std::nullopt;
    }

    return Instruction{
        .opcode = info->opcode,
        .arg = static_cast<std::uint32_t>(args),
    };
}

} // namespace SCM::Codec