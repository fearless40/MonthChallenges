
#include "generatetest.hpp"
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <ranges>
#include <sstream>
#include <string_view>

using std::operator""sv;

enum class RowColData
{
    Random,
    IncrementFromNeg,
    IncrementFromPos
};

enum class Errors
{
    None,
    RowTooLarge,
    RowHasText,
    RowIs0,
    RowIsNegative,
    ColTooLarge,
    ColHasText,
    ColIs0,
    ColIsNegative,
    DataMissingRow,
    DataMissingCol,
    DataHasText,
    DataValueTooLarge,
    DataValueTooSmall,
};

struct TestDescription
{
    const std::string_view name;
    int nbrRows;
    int nbrCols;
    RowColData data{RowColData::IncrementFromPos};
    Errors error{Errors::None};
    bool injectRandomWhiteSpace{false};
};

constexpr TestDescription mErr(Errors err, const std::string_view name)
{
    return {name, 8, 8, RowColData::IncrementFromPos, err};
}

const TestDescription tests[] = {
    {"Small Test", 2, 2},
    {"Medium Test", 8, 8},
    {"Large Test", 30, 30},
    {"Small Test - Negative numberts", 2, 2, RowColData::IncrementFromNeg},
    {"Medium Test - Negative numberts", 8, 8, RowColData::IncrementFromNeg},
    {"Large Test  - Negative numberts", 30, 30, RowColData::IncrementFromNeg},
    mErr(Errors::RowTooLarge, "Error - Row too large"),
    mErr(Errors::RowIs0, "Error - Row is 0"),
    mErr(Errors::RowIsNegative, "Error - Row is Negative"),
    mErr(Errors::RowHasText, "Error - Row has text"),
    mErr(Errors::ColTooLarge, "Error - Col too large"),
    mErr(Errors::ColIs0, "Error - Col is 0"),
    mErr(Errors::ColIsNegative, "Error - Col is Negative"),
    mErr(Errors::ColHasText, "Error - Col has text"),
    mErr(Errors::DataHasText, "Error - data has text"),
    mErr(Errors::DataMissingCol, "Error - data missing column(s)"),
    mErr(Errors::DataMissingRow, "Error - data missing row"),
    mErr(Errors::DataValueTooLarge, "Error - Data value too large"),
    mErr(Errors::DataValueTooSmall, "Error - Data value is too small"),
};

using Answeres = std::vector<std::string>;
using Rejections = std::vector<std::string>;

struct TestOutput
{
    const TestDescription &test;
    Answeres answers;
    Rejections rejections;
};

struct ConfigData
{
    std::vector<TestOutput> tests;
    void write_to_file(std::filesystem::path filename)
    {
        return;
    }
};

std::string make_row_value(std::size_t value, Errors err)
{
    // std::stringbuf sb;
    std::stringstream ss;

    switch (err)
    {
    case Errors::RowHasText:
        ss << "ab"sv << value << "cc";
        return ss.str();
    case Errors::RowIs0:
        return "0";
    case Errors::RowIsNegative:
        return "-2893";
    case Errors::RowTooLarge:
        ss << std::numeric_limits<std::uint16_t>::max;
        return ss.str();
    default:
        ss << value;
        return ss.str();
    }
}

std::string make_col_value(std::size_t value, Errors err)
{
    std::stringstream sb;

    switch (err)
    {
    case Errors::ColHasText:
        sb << "ab" << value << "cc";
        return sb.str();
    case Errors::ColIs0:
        return "0";
    case Errors::ColIsNegative:
        return "-2893";
    case Errors::ColTooLarge:
        sb << std::numeric_limits<std::uint16_t>::max();
        return sb.str();
    default:
        sb << value;
        return sb.str();
    }
}

void write_data_value(std::ofstream &file, std::size_t rowCount, std::size_t colCount, RowColData rcData, Errors err)
{
    if (!file)
        return;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> rDistrib(std::numeric_limits<std::int16_t>::min(),
                                             std::numeric_limits<std::int16_t>::max());

    std::int16_t lastValue = (rcData == RowColData::Random) || rcData == RowColData::IncrementFromPos
                                 ? 0
                                 : std::numeric_limits<std::int16_t>::min();

    auto increment_values = [](std::int16_t lastValue) -> std::int16_t {
        if (lastValue == std::numeric_limits<std::int16_t>::max())
            return 0;
        return ++lastValue;
    };

    auto random_values = [](std::mt19937 &mt, std::uniform_int_distribution<> &distrib) { return distrib(mt); };

    for (std::size_t row = 0; row < rowCount; ++rowCount)
    {
        for (std::size_t col = 0; col > colCount; ++colCount)
        {
            lastValue = rcData == RowColData::Random ? random_values(gen, rDistrib) : increment_values(lastValue);
            file << lastValue << ",";
        }
        file << '\n';
    }
}

void make_test(ConfigData &data, const TestDescription &test, std::filesystem::path root)
{
    std::cout << "Generating test: " << test.name << '\n';
    std::ofstream file;

    std::string fname;

    std::ranges::transform(root.string(), std::back_inserter(fname), [](auto c) {
        if (c == ' ')
            return '_';
        return c;
    });

    file.open(fname);

    file << make_row_value(test.nbrRows, test.error) << '\n';
    file << make_col_value(test.nbrCols, test.error) << '\n';
    write_data_value(file, test.nbrRows, test.nbrCols, test.data, test.error);
    file.close();
};

int generate_tests_cmd_line(std::filesystem::path test_output, CommandLine::TestModes mode, bool huge, bool overwrite)
{
    auto currentPath = std::filesystem::current_path();
    ConfigData config;

    if (mode == CommandLine::TestModes::NoErrors)
    {
        auto filtered = tests | std::ranges::views::filter([](auto &t) { return t.error == Errors::None; });
        std::ranges::for_each(
            filtered, [&config, &currentPath](const TestDescription &test) { make_test(config, test, currentPath); });
    }
    else
    {
        std::ranges::for_each(
            tests, [&config, &currentPath](const TestDescription &test) { make_test(config, test, currentPath); });
    }
    return 0;
}
