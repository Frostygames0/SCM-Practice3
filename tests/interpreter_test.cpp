#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "assembler.hpp"
#include "interpreter.hpp"

using namespace SCM;

namespace
{
std::vector<std::byte> toCode(const Asm::Program& program)
{
    std::vector<std::byte> code;
    for (const auto& instruction : program)
    {
        const auto bytes = Codec::encode(instruction);
        code.insert(code.end(), bytes.begin(), bytes.end());
    }
    return code;
}

std::expected<Vm::State, std::string> runSource(std::string_view source)
{
    const auto program = Asm::assemble(source);
    if (!program)
    {
        return std::unexpected{program.error().message};
    }
    return Vm::run(toCode(*program));
}

std::expected<Vm::State, std::string> runExample(std::string_view name)
{
    const auto source = Asm::readSource(std::filesystem::path{EXAMPLES_DIR} / name);
    if (!source)
    {
        return std::unexpected{source.error().message};
    }
    return runSource(*source);
}

std::vector<std::uint32_t> cells(const Vm::State& state, std::size_t first, std::size_t count)
{
    return {state.data.begin() + static_cast<std::ptrdiff_t>(first),
            state.data.begin() + static_cast<std::ptrdiff_t>(first + count)};
}
} // namespace

TEST(Interpreter, LoadSetsAccumulator)
{
    const auto state = runSource("LOAD 536870911");
    ASSERT_TRUE(state) << state.error();
    EXPECT_EQ(state->accumulator, 536'870'911U);
}

TEST(Interpreter, WriteThenReadRoundTrips)
{
    const auto state = runSource("LOAD 42\nWRITE 100\nLOAD 0\nREAD 100");
    ASSERT_TRUE(state) << state.error();
    EXPECT_EQ(state->data[100], 42U);
    EXPECT_EQ(state->accumulator, 42U);
}

TEST(Interpreter, MinKeepsSmallerValue)
{
    const auto state = runSource("LOAD 5\nWRITE 0\nLOAD 9\nMIN 0\nWRITE 1\nLOAD 3\nMIN 0");
    ASSERT_TRUE(state) << state.error();
    EXPECT_EQ(state->data[1], 5U);
    EXPECT_EQ(state->accumulator, 3U);
}

TEST(Interpreter, EmptyProgramLeavesMemoryZeroed)
{
    const auto state = Vm::run({});
    ASSERT_TRUE(state) << state.error();
    EXPECT_EQ(state->accumulator, 0U);
    EXPECT_EQ(state->data.size(), Vm::DataMemorySize);
}

TEST(Interpreter, RejectsTruncatedProgram)
{
    const std::vector<std::byte> code(7);
    EXPECT_FALSE(Vm::run(code));
}

TEST(Interpreter, RejectsInvalidInstruction)
{
    const std::vector<std::byte> code(Codec::InstructionSize);
    EXPECT_FALSE(Vm::run(code));
}

TEST(Interpreter, RejectsAddressOutsideDataMemory)
{
    EXPECT_FALSE(runSource("READ " + std::to_string(Vm::DataMemorySize)));
    EXPECT_TRUE(runSource("READ " + std::to_string(Vm::DataMemorySize - 1)));
}

TEST(Interpreter, CopyArrayExample)
{
    const auto state = runExample("copy_array.asm");
    ASSERT_TRUE(state) << state.error();
    const std::vector<std::uint32_t> expected{11, 22, 33, 44, 55};
    EXPECT_EQ(cells(*state, 0, 5), expected);
    EXPECT_EQ(cells(*state, 10, 5), expected);
}

TEST(Interpreter, MinExample)
{
    const auto state = runExample("min.asm");
    ASSERT_TRUE(state) << state.error();
    EXPECT_EQ(cells(*state, 0, 4), (std::vector<std::uint32_t>{500, 70, 70, 70}));
}

TEST(Interpreter, VectorMinExamples)
{
    const struct
    {
        std::string_view file;
        std::vector<std::uint32_t> expected;
    } cases[] = {
        {.file = "vector_min_1.asm", .expected = {10, 143, 143, 0, 143}   },
        {.file = "vector_min_2.asm", .expected = {143, 143, 143, 143, 100}},
        {.file = "vector_min_3.asm", .expected = {142, 143, 143, 0, 143}  },
    };

    for (const auto& [file, expected] : cases)
    {
        SCOPED_TRACE(file);
        const auto state = runExample(file);
        ASSERT_TRUE(state) << state.error();
        EXPECT_EQ(cells(*state, 0, 5), expected);
    }
}

TEST(Dump, FormatsRangeAsXml)
{
    Vm::State state;
    state.data[3] = 7;
    state.data[4] = 8;

    const auto xml = Vm::toXml(state, {.first = 3, .last = 4});
    ASSERT_TRUE(xml) << xml.error();
    EXPECT_EQ(*xml,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<memory>\n"
              "  <cell address=\"3\">7</cell>\n"
              "  <cell address=\"4\">8</cell>\n"
              "</memory>\n");
}

TEST(Dump, RejectsInvalidRange)
{
    const Vm::State state;
    EXPECT_FALSE(Vm::toXml(state, {.first = 5, .last = 4}));
    EXPECT_FALSE(Vm::toXml(state, {.first = 0, .last = Vm::DataMemorySize}));
}
