#pragma once
#include <cstdint>
#include <format>
#include <string>
#include <string_view>

struct RowCol
{
    std::uint16_t row;
    std::uint16_t col;

    auto operator<=>(const RowCol &other) const = default;

    std::string excel() const;

    std::string colrow() const;

    static RowCol random(std::uint16_t maxRow, std::uint16_t maxCol);
};
