#pragma once
#include "RowCol.hpp"
#include <cstdint>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace Tests
{

enum class RowColDataGeneration
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

struct QueryAnswer
{
    bool is_oob;
    RowCol pos;
    std::int16_t answer;
};

using Queries = std::vector<RowCol>;
using QueryAnswers = std::vector<QueryAnswer>;

class Definition
{
  public:
    std::string mName;
    std::uint16_t mNbrRows;
    std::uint16_t mNbrCols;
    RowColDataGeneration mData{RowColDataGeneration::IncrementFromPos};
    Errors mError{Errors::None};
    bool mInjectRandomWhiteSpace{false};

    bool huge() const
    {
        return mNbrRows == std::numeric_limits<std::uint16_t>::max() &&
               mNbrCols == std::numeric_limits<std::uint16_t>::max();
    }

    QueryAnswers generate(std::ostream &file, const std::vector<RowCol> &guesses) const;

  private:
    std::string make_row_value(std::size_t value, Errors err) const;
    std::string make_col_value(std::size_t value, Errors err) const;
    QueryAnswers write_data_value(std::ostream &file, const std::vector<RowCol> &guesses) const;
};

} // namespace Tests