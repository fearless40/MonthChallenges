#pragma once
#include <compare>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>

struct RowCol
{
    std::uint16_t row;
    std::uint16_t col;

    auto operator<=>(const RowCol &other) const = default;

    std::string as_base26_fmt() const;

    std::string as_colrow_fmt() const;

    static RowCol random(std::uint16_t maxRow, std::uint16_t maxCol);

    static RowCol from_string(std::string_view const);
};
