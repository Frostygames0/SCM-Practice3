#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace SCM::Codec
{

constexpr std::size_t InstructionSize = 5;
constexpr unsigned OpcodeBits = 4;
constexpr unsigned ConstantBits = 29;
constexpr unsigned AddressBits = 24;

static_assert(OpcodeBits + ConstantBits <= InstructionSize * 8);
static_assert(ConstantBits < 32 && AddressBits < 32);

enum class Opcode : std::uint8_t
{
    Load = 7,
    Read = 6,
    Write = 9,
    Min = 4,
};

struct OpcodeInfo
{
    Opcode opcode;
    std::string_view mnemonic;
    unsigned argBits;
};

inline constexpr OpcodeInfo OpcodeTable[] = {
    {.opcode = Opcode::Load,  .mnemonic = "LOAD",  .argBits = ConstantBits},
    {.opcode = Opcode::Read,  .mnemonic = "READ",  .argBits = AddressBits },
    {.opcode = Opcode::Write, .mnemonic = "WRITE", .argBits = AddressBits },
    {.opcode = Opcode::Min,   .mnemonic = "MIN",   .argBits = AddressBits },
};

constexpr const OpcodeInfo* findInfo(Opcode op)
{
    for (const auto& info : OpcodeTable)
    {
        if (info.opcode == op)
        {
            return &info;
        }
    }
    return nullptr;
}

constexpr const OpcodeInfo* findInfo(std::string_view mnemonic)
{
    for (const auto& info : OpcodeTable)
    {
        if (info.mnemonic == mnemonic)
        {
            return &info;
        }
    }
    return nullptr;
}

constexpr bool argsFit(Opcode op, std::uint32_t args)
{
    const auto* info = findInfo(op);
    return info != nullptr && (args >> info->argBits) == 0;
}

struct Instruction
{
    Opcode opcode;
    std::uint32_t arg;
};

using EncodedBytes = std::array<std::byte, InstructionSize>;

EncodedBytes encode(const Instruction& instruction);
std::optional<Instruction> decode(const EncodedBytes& bytes);

} // namespace SCM::Codec
