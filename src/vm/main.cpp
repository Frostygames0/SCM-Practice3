#include <CLI/CLI.hpp>

#include <cstdint>
#include <iostream>
#include <utility>

#include "interpreter.hpp"

int main(int argc, char** argv)
{
    CLI::App app{"Interpreter for the SCM educational virtual machine"};

    std::filesystem::path programPath;
    app.add_option("-p,--program", programPath, "Path to the assembled binary file")
        ->check(CLI::ExistingFile)
        ->required();

    std::filesystem::path dumpPath;
    app.add_option("-d,--dump", dumpPath, "Path to the XML memory dump file")->required();

    std::pair<std::uint32_t, std::uint32_t> range;
    app.add_option("-r,--range", range, "First and last data memory address to dump (inclusive)")
        ->required();

    CLI11_PARSE(app, argc, argv);

    auto code = SCM::Vm::readBinary(programPath);
    if (!code)
    {
        std::cerr << code.error() << '\n';
        return 1;
    }

    auto state = SCM::Vm::run(*code);
    if (!state)
    {
        std::cerr << programPath.string() << ": " << state.error() << '\n';
        return 1;
    }

    const SCM::Vm::Range dumpRange{.first = range.first, .last = range.second};
    if (auto dumped = SCM::Vm::writeDump(dumpPath, *state, dumpRange); !dumped)
    {
        std::cerr << dumped.error() << '\n';
        return 1;
    }
    return 0;
}
