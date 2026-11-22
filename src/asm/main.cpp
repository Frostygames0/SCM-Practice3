#include <CLI/CLI.hpp>

#include <cstddef>
#include <format>
#include <iostream>
#include <string>

#include "assembler.hpp"

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
} // namespace

int main(int argc, char** argv)
{
    CLI::App app{"Assembler for the SCM educational virtual machine"};

    std::filesystem::path codePath;
    app.add_option("-c,--code", codePath, "Path to the source file to assemble")
        ->check(CLI::ExistingFile)
        ->required();

    std::filesystem::path assembledPath;
    app.add_option("-r,--result", assembledPath, "Path to the output binary file")->required();

    bool testMode = false;
    app.add_flag("-t,--test", testMode, "Print the machine code of every instruction");

    CLI11_PARSE(app, argc, argv);

    auto source = SCM::Asm::readSource(codePath);
    if (!source)
    {
        std::cerr << source.error() << '\n';
        return 1;
    }

    auto program = SCM::Asm::assemble(*source);
    if (!program)
    {
        std::cerr << codePath.string() << ": " << program.error() << '\n';
        return 1;
    }

    if (auto written = SCM::Asm::writeBinary(assembledPath, *program); !written)
    {
        std::cerr << written.error() << '\n';
        return 1;
    }

    if (testMode)
    {
        for (const auto& instruction : *program)
        {
            std::cout << toHex(SCM::Codec::encode(instruction)) << '\n';
        }
    }

    std::cout << "Binary size: " << program->size() * SCM::Codec::InstructionSize << " bytes\n";
    return 0;
}
