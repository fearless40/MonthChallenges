#include "RowCol.hpp"
#include "RandomUtil.hpp"
#include <algorithm>
#include <charconv>
#include <ranges>
#include <utility>

std::string RowCol::as_excel_fmt() const
{
    std::string result{};
    result.reserve(10);

    if (col == 0)
    {
        result = "a";
    }
    else
    {
        auto c = col;
        while (c > 0)
        {
            auto out = std::div(c, 26);
            result += static_cast<char>('a' + out.rem);
            c = out.quot;
        }
    }
    std::ranges::reverse(result);
    result.append(std::to_string(row));
    return result;
}

std::string RowCol::as_colrow_fmt() const
{
    return std::format("{},{}", col, row);
}

RowCol RowCol::random(std::uint16_t maxRow, std::uint16_t maxCol)
{
    return {Random::between<std::uint16_t>(0, maxRow), Random::between<std::uint16_t>(0, maxCol)};
}

bool from_chars(std::string_view const value, std::uint16_t &out)
{
    auto err = std::from_chars(value.data(), value.data() + value.size(), out);
    if (err.ec == std::errc())
        return true;
    return false;
}

RowCol parse_comma_fmt(const std::string_view value)
{
    auto pos = value.find_first_of(",");
    auto colText = value.substr(0, pos);
    auto rowText = value.substr(pos + 1);

    if (colText.size() == 0 || rowText.size() == 0)
    {
        return {};
    }

    RowCol ret;

    if (from_chars(colText, ret.col) && from_chars(rowText, ret.row))
    {
        return ret;
    }
    return {};
}

RowCol RowCol::from_string(std::string_view const value)
{
    // Check for comma format
    {
        auto pos = value.find_first_of(",");
        if (pos != std::string_view::npos)
            return parse_comma_fmt(value);
    }

    auto pos = value.find_first_of("0123456789");
    if (pos == std::string_view::npos)
    {
        return {};
    }

    auto letters = value.substr(0, pos);

    // Convert all letters to lowercase
    auto toLower = [](auto l) { return std::tolower(static_cast<unsigned char>(l)); };

    std::size_t numberLetters = letters.size();
    std::size_t col = 0;
    for (auto l : letters | std::views::transform(toLower))
    {
        col += (l - 'a') * std::pow(26, numberLetters - 1);
        --numberLetters;
    }

    RowCol ret;

    if (from_chars(value.substr(pos), ret.row))
    {
        ret.col = static_cast<std::uint16_t>(col);
        return ret;
    }

    return {};
}