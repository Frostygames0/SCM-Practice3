#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>

#include <CLI/CLI.hpp>

#include "assembler.hpp"
#include "codec.hpp"

namespace
{
std::string toHex(const SCM::Codec::EncodedBytes& bytes)
{
    std::string out;
    for (std::size_t i{}; i < bytes.size(); ++i)
    {
        out += std::format("{}0x{:02X}", i == 0 ? "" : ", ", std::to_integer<unsigned>(bytes[i]));
    }
    return out;
}

template <typename E>
int fail(const E& error)
{
    std::cerr << error << '\n';
    return EXIT_FAILURE;
}
} // namespace

int main(int argc, char** argv)
{
    CLI::App app{"Assembler for the SCM educational virtual machine"};

    std::filesystem::path inputPath;
    app.add_option("-i,--input", inputPath, "Path to the source file to assemble")
        ->check(CLI::ExistingFile)
        ->required();

    std::filesystem::path resultPath;
    app.add_option("-r,--result", resultPath, "Path to the output binary file")->required();

    bool testMode{};
    app.add_flag("-t,--test", testMode, "Print the machine code of every instruction");

    CLI11_PARSE(app, argc, argv);

    using namespace SCM;

    const auto source = Asm::readSource(inputPath);
    if (!source)
    {
        return fail(source.error());
    }

    const auto program = Asm::assemble(*source);
    if (!program)
    {
        return fail(program.error());
    }

    if (const auto written = Asm::writeBinary(resultPath, *program); !written)
    {
        return fail(written.error());
    }

    if (testMode)
    {
        for (const auto& instruction : *program)
        {
            std::cout << toHex(Codec::encode(instruction)) << '\n';
        }
    }

    const auto binarySize = program->size() * Codec::InstructionSize;
    std::cout << "Binary size: " << binarySize << " bytes\n";

    return EXIT_SUCCESS;
}
