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

struct QueryErrorAnswer
{
    bool is_error;
};

struct QueryCorrectAnswer
{
    RowCol pos;
    std::int16_t answer;
};

using QueryAnswer = std::variant<QueryCorrectAnswer, QueryErrorAnswer>;
using Queries = std::vector<RowCol>;
using QueryAnswers = std::vector<QueryAnswer>;

class Definition
{
  public:
    const std::string mName;
    const std::uint16_t mNbrRows;
    const std::uint16_t mNbrCols;
    const RowColDataGeneration mData{RowColDataGeneration::IncrementFromPos};
    const Errors mError{Errors::None};
    const bool mInjectRandomWhiteSpace{false};

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