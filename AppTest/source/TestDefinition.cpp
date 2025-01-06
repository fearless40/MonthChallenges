#include "TestDefinition.hpp"
#include <algorithm>
#include <random>
#include <sstream>

using std::operator""sv;

namespace Tests
{

QueryAnswers Definition::generate(std::ostream &file, const std::vector<RowCol> &guesses) const
{

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> bool_random(0, 1);
    std::uniform_int_distribution<> count_random(1, 4);

    auto injectWhiteSpace = [&](std::size_t count = 1) {
        char ws = (bool_random(gen) == 0 ? '\t' : ' ');
        for (int i = 0; i < count; ++i)
            file << ws;
    };

    if (mInjectRandomWhiteSpace)
        injectWhiteSpace(count_random(gen));

    file << make_row_value(mNbrRows, mError) << '\n';

    if (mInjectRandomWhiteSpace)
        injectWhiteSpace(count_random(gen));

    file << make_col_value(mNbrCols, mError) << '\n';

    return write_data_value(file, guesses);
};

std::string Definition::make_row_value(std::size_t value, Errors err) const
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
        ss << std::numeric_limits<std::uint16_t>::max() + 5;
        return ss.str();
    default:
        ss << value;
        return ss.str();
    }
}

std::string Definition::make_col_value(std::size_t value, Errors err) const
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
        sb << std::numeric_limits<std::uint16_t>::max() + 5;
        return sb.str();
    default:
        sb << value;
        return sb.str();
    }
}

QueryAnswers Definition::write_data_value(std::ostream &file, const std::vector<RowCol> &guesses) const
{
    if (!file)
        return {};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::int16_t> dataRandom(std::numeric_limits<std::int16_t>::min(),
                                                           std::numeric_limits<std::int16_t>::max());
    std::uniform_int_distribution<> boolGenerator(0, 1);
    std::uniform_int_distribution<std::size_t> random_row(0, mNbrRows);
    std::uniform_int_distribution<std::size_t> random_col(0, mNbrCols);

    auto bool_random = [&boolGenerator, &gen]() -> bool {
        if (boolGenerator(gen) == 0)
            return false;
        return true;
    };

    auto row_random = [&random_row, &gen]() { return random_row(gen); };

    auto col_random = [&random_col, &gen]() { return random_col(gen); };

    auto increment_values = [](std::int16_t lastValue) -> std::int16_t {
        if (lastValue == std::numeric_limits<std::int16_t>::max())
            return 0;
        return ++lastValue;
    };

    auto random_values = [&]() -> std::int16_t { return dataRandom(gen); };

    auto injectWhiteSpace = [&](std::size_t count = 1) {
        char ws = (bool_random() == true ? '\t' : ' ');
        for (int i = 0; i < count; ++i)
            file << ws;
    };

    std::int16_t lastValue = (mData == RowColDataGeneration::Random) || mData == RowColDataGeneration::IncrementFromPos
                                 ? 0
                                 : std::numeric_limits<std::int16_t>::min();

    const std::size_t rowToGen = mError == Errors::DataMissingRow ? mNbrRows - 1 : mNbrRows;
    bool skipOneColumn = mError == Errors::DataMissingCol ? true : false;
    std::size_t randCol = col_random();
    std::size_t randRow = row_random();

    QueryAnswers answers;
    answers.reserve(guesses.size());

    for (std::size_t row = 0; row < rowToGen; ++row)
    {
        for (std::size_t col = 0; col < mNbrCols; ++col)
        {
            if (skipOneColumn && randCol == col)
            {
                skipOneColumn = false;
                continue;
            }

            if (col != 0)
                file << ',';

            lastValue = mData == RowColDataGeneration::Random ? random_values() : increment_values(lastValue);

            if (mInjectRandomWhiteSpace && bool_random())
                injectWhiteSpace(row_random());

            if (row == randRow && mError != Errors::None && col == randCol)
            {
                if (mError == Errors::DataValueTooLarge)
                    file << std::numeric_limits<std::int16_t>::max() + random_values();
                else if (mError == Errors::DataValueTooSmall)
                    file << static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::min() - random_values());
                else if (mError == Errors::DataHasText)
                    file << "ab1234df";
            }
            else
            {
                if (mError == Errors::None)
                {
                    RowCol current{static_cast<std::uint16_t>(row), static_cast<std::uint16_t>(col)};
                    auto it = std::ranges::find(guesses, current);
                    if (it != guesses.end())
                    {
                        QueryAnswer q{false, current, lastValue};
                        answers.emplace_back(q);
                    }
                }
                file << lastValue;
            }

            if (mInjectRandomWhiteSpace && bool_random())
                injectWhiteSpace(row_random());
        }
        file << '\n';
    }

    if (mError != Errors::None)
    {
        return {};
    }
    return answers;
}

} // namespace Tests