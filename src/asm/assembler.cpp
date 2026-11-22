#include "assembler.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <iterator>
#include <system_error>

namespace SCM::Asm
{
namespace
{

constexpr std::string_view Whitespace = " \t\r\v\f";
constexpr std::string_view Utf8Bom = "\xEF\xBB\xBF";

std::string_view trim(std::string_view s)
{
    const auto first = s.find_first_not_of(Whitespace);
    if (first == std::string_view::npos)
    {
        return {};
    }
    const auto last = s.find_last_not_of(Whitespace);
    return s.substr(first, last - first + 1);
}

std::string_view nextToken(std::string_view& s)
{
    s = trim(s);
    const auto end = std::min(s.find_first_of(Whitespace), s.size());
    const auto token = s.substr(0, end);
    s.remove_prefix(end);
    return token;
}

std::string toUpper(std::string_view s)
{
    std::string out(s);
    std::ranges::transform(
        out, out.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return out;
}

std::expected<std::uint32_t, std::string> parseNumber(std::string_view token)
{
    int base = 10;
    if (token.size() > 2 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X'))
    {
        base = 16;
        token.remove_prefix(2);
    }

    std::uint32_t value{};
    const auto* begin = token.data();
    const auto* end = token.data() + token.size();
    const auto [ptr, ec] = std::from_chars(begin, end, value, base);

    if (ec == std::errc::result_out_of_range)
    {
        return std::unexpected{"number out of range"};
    }
    if (ec != std::errc{} || ptr != end)
    {
        return std::unexpected{"invalid number"};
    }
    return value;
}

std::expected<Codec::Instruction, std::string> parseInstruction(std::string_view line)
{
    const auto mnemonicToken = nextToken(line);
    const auto mnemonic = toUpper(mnemonicToken);

    const auto* info = Codec::findInfo(mnemonic);
    if (info == nullptr)
    {
        return std::unexpected{"unknown mnemonic '" + std::string(mnemonicToken) + "'"};
    }

    const auto argToken = nextToken(line);
    if (argToken.empty())
    {
        return std::unexpected{std::string(info->mnemonic) + " expects an argument"};
    }

    if (!trim(line).empty())
    {
        return std::unexpected{"unexpected trailing input '" + std::string(trim(line)) + "'"};
    }

    auto value = parseNumber(argToken);
    if (!value)
    {
        return std::unexpected{value.error() + " '" + std::string(argToken) + "'"};
    }

    if (!Codec::argsFit(info->opcode, *value))
    {
        return std::unexpected{"argument " + std::to_string(*value) + " doesn't fit in " +
                               std::to_string(info->argBits) + " bits for " +
                               std::string(info->mnemonic)};
    }

    return Codec::Instruction{.opcode = info->opcode, .args = *value};
}

} // namespace

std::expected<Program, Error> assemble(std::string_view text)
{
    if (text.starts_with(Utf8Bom))
    {
        text.remove_prefix(Utf8Bom.size());
    }

    Program program;
    std::size_t lineNo = 0;

    while (!text.empty())
    {
        ++lineNo;

        const auto newline = std::min(text.find('\n'), text.size());
        auto line = text.substr(0, newline);
        text.remove_prefix(std::min(newline + 1, text.size()));

        if (const auto comment = line.find(';'); comment != std::string_view::npos)
        {
            line = line.substr(0, comment);
        }

        line = trim(line);
        if (line.empty())
        {
            continue;
        }

        auto instruction = parseInstruction(line);
        if (!instruction)
        {
            return std::unexpected{
                Error{.line = lineNo, .message = std::move(instruction.error())}
            };
        }
        program.push_back(*instruction);
    }

    return program;
}

std::expected<std::string, Error> readSource(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        return std::unexpected{
            Error{.line = 0, .message = "cannot open '" + path.string() + "'"}
        };
    }

    std::string text{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    if (in.bad())
    {
        return std::unexpected{
            Error{.line = 0, .message = "failed to read '" + path.string() + "'"}
        };
    }
    return text;
}

std::expected<void, Error> writeBinary(const std::filesystem::path& path, const Program& program)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        return std::unexpected{
            Error{.line = 0, .message = "cannot open '" + path.string() + "'"}
        };
    }

    for (const auto& instruction : program)
    {
        const auto bytes = Codec::encode(instruction);
        out.write(reinterpret_cast<const char*>(bytes.data()),
                  static_cast<std::streamsize>(bytes.size()));
    }

    out.flush();
    if (!out)
    {
        return std::unexpected{
            Error{.line = 0, .message = "failed to write '" + path.string() + "'"}
        };
    }
    return {};
}

} // namespace SCM::Asm
