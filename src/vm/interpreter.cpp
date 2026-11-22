#include "interpreter.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <iterator>

namespace SCM::Vm
{

std::expected<void, std::string> execute(State& state, const Codec::Instruction& instruction)
{
    using Codec::Opcode;

    const auto address = instruction.args;
    if (instruction.opcode != Opcode::Load && address >= state.data.size())
    {
        return std::unexpected{
            std::format("address {} is out of data memory (size {})", address, state.data.size())};
    }

    switch (instruction.opcode)
    {
        case Opcode::Load:
            state.accumulator = instruction.args;
            break;
        case Opcode::Read:
            state.accumulator = state.data[address];
            break;
        case Opcode::Write:
            state.data[address] = state.accumulator;
            break;
        case Opcode::Min:
            state.accumulator = std::min(state.accumulator, state.data[address]);
            break;
    }
    return {};
}

std::expected<State, std::string> run(std::span<const std::byte> code)
{
    if (code.size() % Codec::InstructionSize != 0)
    {
        return std::unexpected{std::format(
            "program size {} is not a multiple of {} bytes", code.size(), Codec::InstructionSize)};
    }

    State state;
    for (std::size_t pc{}; pc < code.size(); pc += Codec::InstructionSize)
    {
        Codec::EncodedBytes bytes{};
        std::ranges::copy(code.subspan(pc, Codec::InstructionSize), bytes.begin());

        const auto instruction = Codec::decode(bytes);
        if (!instruction)
        {
            return std::unexpected{std::format("offset {}: invalid instruction", pc)};
        }

        if (auto result = execute(state, *instruction); !result)
        {
            return std::unexpected{std::format("offset {}: {}", pc, result.error())};
        }
    }
    return state;
}

std::expected<std::string, std::string> toXml(const State& state, Range range)
{
    if (range.first > range.last || range.last >= state.data.size())
    {
        return std::unexpected{std::format("invalid dump range {}..{} (data memory size {})",
                                           range.first,
                                           range.last,
                                           state.data.size())};
    }

    std::string out = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<memory>\n";
    for (std::size_t address = range.first; address <= range.last; ++address)
    {
        out += std::format("  <cell address=\"{}\">{}</cell>\n", address, state.data[address]);
    }
    out += "</memory>\n";
    return out;
}

std::expected<std::vector<std::byte>, std::string> readBinary(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        return std::unexpected{"cannot open '" + path.string() + "'"};
    }

    std::vector<char> raw{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    if (in.bad())
    {
        return std::unexpected{"failed to read '" + path.string() + "'"};
    }

    std::vector<std::byte> code(raw.size());
    std::ranges::transform(raw, code.begin(), [](char c) { return static_cast<std::byte>(c); });
    return code;
}

std::expected<void, std::string> writeDump(const std::filesystem::path& path,
                                           const State& state,
                                           Range range)
{
    const auto xml = toXml(state, range);
    if (!xml)
    {
        return std::unexpected{xml.error()};
    }

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        return std::unexpected{"cannot open '" + path.string() + "'"};
    }

    out << *xml;
    out.flush();
    if (!out)
    {
        return std::unexpected{"failed to write '" + path.string() + "'"};
    }
    return {};
}

} // namespace SCM::Vm
