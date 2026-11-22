#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "codec.hpp"

using namespace SCM::Codec;

namespace
{
EncodedBytes bytes(std::vector<std::uint8_t> values)
{
    EncodedBytes out{};
    for (std::size_t i{}; i < out.size(); ++i)
    {
        out[i] = static_cast<std::byte>(values[i]);
    }
    return out;
}

struct Vector
{
    Instruction instruction;
    EncodedBytes encoded;
};

const std::vector<Vector> SpecVectors = {
    {
     .instruction = {.opcode = Opcode::Load, .arg = 968},
     .encoded = bytes({0x87, 0x3C, 0x00, 0x00, 0x00}),
     },
    {
     .instruction = {.opcode = Opcode::Read, .arg = 341},
     .encoded = bytes({0x56, 0x15, 0x00, 0x00, 0x00}),
     },
    {
     .instruction = {.opcode = Opcode::Write, .arg = 554},
     .encoded = bytes({0xA9, 0x22, 0x00, 0x00, 0x00}),
     },
    {
     .instruction = {.opcode = Opcode::Min, .arg = 618},
     .encoded = bytes({0xA4, 0x26, 0x00, 0x00, 0x00}),
     },
};
} // namespace

TEST(Codec, EncodesSpecVectors)
{
    for (const auto& [instruction, encoded] : SpecVectors)
    {
        EXPECT_EQ(encode(instruction), encoded);
    }
}

TEST(Codec, DecodesSpecVectors)
{
    for (const auto& [instruction, encoded] : SpecVectors)
    {
        const auto decoded = decode(encoded);
        ASSERT_TRUE(decoded);
        EXPECT_EQ(decoded->opcode, instruction.opcode);
        EXPECT_EQ(decoded->arg, instruction.arg);
    }
}

TEST(Codec, RoundTripsMaximumArguments)
{
    for (const Instruction instruction : {
             Instruction{.opcode = Opcode::Load, .arg = (1U << ConstantBits) - 1},
             Instruction{.opcode = Opcode::Min,  .arg = (1U << AddressBits) - 1 },
    })
    {
        const auto decoded = decode(encode(instruction));
        ASSERT_TRUE(decoded);
        EXPECT_EQ(decoded->arg, instruction.arg);
    }
}

TEST(Codec, RejectsUnknownOpcode)
{
    EXPECT_FALSE(decode(bytes({0x00, 0x00, 0x00, 0x00, 0x00})));
    EXPECT_FALSE(decode(bytes({0x0F, 0x00, 0x00, 0x00, 0x00})));
}

TEST(Codec, RejectsBitsBeyondArgumentField)
{
    EXPECT_FALSE(decode(bytes({0x07, 0x00, 0x00, 0x00, 0x02})));
    EXPECT_FALSE(decode(bytes({0x06, 0x00, 0x00, 0x10, 0x00})));
    EXPECT_FALSE(decode(bytes({0x04, 0x00, 0x00, 0x00, 0x80})));
}

TEST(Codec, ArgsFitChecksPerOpcodeWidth)
{
    EXPECT_TRUE(argsFit(Opcode::Load, (1U << ConstantBits) - 1));
    EXPECT_FALSE(argsFit(Opcode::Load, 1U << ConstantBits));
    EXPECT_TRUE(argsFit(Opcode::Write, (1U << AddressBits) - 1));
    EXPECT_FALSE(argsFit(Opcode::Write, 1U << AddressBits));
}
