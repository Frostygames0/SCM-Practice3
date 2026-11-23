#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <utility>

#include <CLI/CLI.hpp>

#include "interpreter.hpp"

namespace
{
template <typename E>
int fail(const E& error)
{
    std::cerr << error << '\n';
    return EXIT_FAILURE;
}
} // namespace

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

    using namespace SCM;

    const auto code = Vm::readBinary(programPath);
    if (!code)
    {
        return fail(code.error());
    }

    const auto state = Vm::run(*code);
    if (!state)
    {
        return fail(state.error());
    }

    const SCM::Vm::Range dumpRange{.first = range.first, .last = range.second};
    if (auto dumped = Vm::writeDump(dumpPath, *state, dumpRange); !dumped)
    {
        return fail(dumped.error());
    }

    return EXIT_SUCCESS;
}
