#include "RowCol.hpp"
#include "RandomUtil.hpp"

std::string RowCol::excel() const;
{
    const std::string_view letters{"abcdefghijklmnopqrstuvwxyz"};
}

std::string RowCol::colrow() const;
{
    return std::format("{},{}", col, row);
}

RowCol RowCol::random(std::uint16_t maxRow, std::uint16_t maxCol);
{
    return {Rand::between<std::uint16_t>(0, maxRow), Rand::between<std::uint16_t>(0, maxCol)};
}