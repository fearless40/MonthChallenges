#include "RowCol.hpp"
#include "RandomUtil.hpp"

std::string RowCol::as_excel_fmt() const
{
    const std::string_view letters{"abcdefghijklmnopqrstuvwxyz"};
    return "";
}

std::string RowCol::as_colrow_fmt() const
{
    return std::format("{},{}", col, row);
}

RowCol RowCol::random(std::uint16_t maxRow, std::uint16_t maxCol)
{
    return {Random::between<std::uint16_t>(0, maxRow), Random::between<std::uint16_t>(0, maxCol)};
}