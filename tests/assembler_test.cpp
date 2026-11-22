#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "assembler.hpp"

using namespace SCM;

namespace
{
std::size_t errorLine(std::string_view source)
{
    const auto result = Asm::assemble(source);
    EXPECT_FALSE(result) << "expected an error for: " << source;
    return result ? 0 : result.error().line;
}
} // namespace

TEST(Assembler, ParsesAllInstructions)
{
    const auto program = Asm::assemble("LOAD 968\nREAD 341\nWRITE 554\nMIN 618\n");
    ASSERT_TRUE(program) << program.error();
    ASSERT_EQ(program->size(), 4U);
    EXPECT_EQ((*program)[0].opcode, Codec::Opcode::Load);
    EXPECT_EQ((*program)[0].arg, 968U);
    EXPECT_EQ((*program)[3].opcode, Codec::Opcode::Min);
    EXPECT_EQ((*program)[3].arg, 618U);
}

TEST(Assembler, IgnoresCommentsBlankLinesAndWhitespace)
{
    const auto program = Asm::assemble("; header\n\n   \t\nLOAD 1 ; trailing\n\tREAD\t2\n;\n");
    ASSERT_TRUE(program) << program.error();
    EXPECT_EQ(program->size(), 2U);
}

TEST(Assembler, HandlesWindowsLineEndingsAndBom)
{
    const auto program = Asm::assemble("\xEF\xBB\xBFLOAD 1\r\nREAD 2\r\n");
    ASSERT_TRUE(program) << program.error();
    ASSERT_EQ(program->size(), 2U);
    EXPECT_EQ((*program)[1].arg, 2U);
}

TEST(Assembler, AcceptsLowercaseAndHex)
{
    const auto program = Asm::assemble("load 0x3C8");
    ASSERT_TRUE(program) << program.error();
    EXPECT_EQ((*program)[0].arg, 968U);
}

TEST(Assembler, AcceptsMaximumArguments)
{
    EXPECT_TRUE(Asm::assemble("LOAD 536870911\nREAD 16777215"));
}

TEST(Assembler, EmptySourceGivesEmptyProgram)
{
    const auto program = Asm::assemble("");
    ASSERT_TRUE(program);
    EXPECT_TRUE(program->empty());
}

TEST(Assembler, ReportsErrorsWithLineNumbers)
{
    EXPECT_EQ(errorLine("LOAD 1\nJUMP 2"), 2U);
    EXPECT_EQ(errorLine("LOAD"), 1U);
    EXPECT_EQ(errorLine("LOAD 1 2"), 1U);
    EXPECT_EQ(errorLine("LOAD -1"), 1U);
    EXPECT_EQ(errorLine("LOAD 12abc"), 1U);
    EXPECT_EQ(errorLine("LOAD 99999999999"), 1U);
    EXPECT_EQ(errorLine("\n\nLOAD 536870912"), 3U);
    EXPECT_EQ(errorLine("WRITE 16777216"), 1U);
}

TEST(Assembler, SpecFileProducesSpecBytes)
{
    const auto source = Asm::readSource(std::filesystem::path{EXAMPLES_DIR} / "spec.asm");
    ASSERT_TRUE(source) << source.error();
    const auto program = Asm::assemble(*source);
    ASSERT_TRUE(program) << program.error();

    const auto output = std::filesystem::temp_directory_path() / "scm3_spec_test.bin";
    ASSERT_TRUE(Asm::writeBinary(output, *program));

    std::ifstream in(output, std::ios::binary);
    const std::vector<unsigned char> actual{std::istreambuf_iterator<char>{in},
                                            std::istreambuf_iterator<char>{}};
    in.close();
    std::filesystem::remove(output);

    const std::vector<unsigned char> expected{
        0x87, 0x3C, 0x00, 0x00, 0x00, 0x56, 0x15, 0x00, 0x00, 0x00,
        0xA9, 0x22, 0x00, 0x00, 0x00, 0xA4, 0x26, 0x00, 0x00, 0x00,
    };
    EXPECT_EQ(actual, expected);
}
