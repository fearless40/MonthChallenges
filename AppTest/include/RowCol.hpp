#pragma once
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <compare>

struct RowCol
{
    std::uint16_t row;
    std::uint16_t col;

    auto operator<=>(const RowCol &other) const = default;

    std::string as_excel_fmt() const;

    std::string as_colrow_fmt() const;

    static RowCol random(std::uint16_t maxRow, std::uint16_t maxCol);
};
