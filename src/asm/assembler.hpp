#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "codec.hpp"

namespace SCM::Asm
{

struct Error
{
    std::size_t line;
    std::string message;

    friend std::ostream& operator<<(std::ostream& out, const Error& error)
    {
        if (error.line != 0)
        {
            out << "line " << error.line << ": ";
        }
        return out << error.message;
    }
};

using Program = std::vector<Codec::Instruction>;

std::expected<Program, Error> assemble(std::string_view text);

std::expected<std::string, Error> readSource(const std::filesystem::path& path);
std::expected<void, Error> writeBinary(const std::filesystem::path& path, const Program& program);

} // namespace SCM::Asm
