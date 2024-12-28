#include "TestConfig.hpp"
#include "RandomUtil.hpp"
#include <algorithm>
#include <iostream>
#include <ostream>
#include <ranges>

namespace Tests
{

#define mErr(err, name) {name, 8, 8, RowColDataGeneration::IncrementFromPos, err}

const Definition tests[] = {
    {"Small Test", 2, 2},
    {"Medium Test", 8, 8},
    {"Large Test", 30, 30},
    {"Row Col Diff", 2, 8},
    {"Col Row Diff", 8, 2},
    {"Small Test - Negative numbers", 2, 2, RowColDataGeneration::IncrementFromNeg},
    {"Medium Test - Negative numbers", 8, 8, RowColDataGeneration::IncrementFromNeg},
    {"Large Test  - Negative numbers", 30, 30, RowColDataGeneration::IncrementFromNeg},
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

#undef mErr

void Configuration::create(const Definition &test, std::size_t nbrQueries, std::size_t outofbounds)
{
    ExpectedResults t{test};

    if (test.mError != Errors::None)
    {
        t.expected.emplace_back(QueryErrorAnswer{true});
        mTests.emplace_back(t);
        return;
    }

    // If there are no errors ensure that a report of error is rejected
    t.rejected.emplace_back("error");

    // Generate queries
    for (int queryCount = 0; queryCount < nbrQueries; ++queryCount)
    {
        t.queries.emplace_back(Random::between<std::uint16_t>(0, test.mNbrRows),
                               Random::between<std::uint16_t>(0, test.mNbrCols));
    }

    // Generate Out of bound queries
    for (std::size_t oobCount = 0; oobCount < outofbounds; ++oobCount)
    {
        t.queries.emplace_back(Random::between<std::uint16_t>(test.mNbrRows + 1, test.mNbrRows * 30),
                               Random::between<std::uint16_t>(test.mNbrCols + 1, test.mNbrCols * 30));
    }

    mTests.emplace_back(t);
}

void Configuration::write_all_tests(std::filesystem::path locationToWrite, bool locateTestFileInSeperateFolder)
{
    if (locateTestFileInSeperateFolder)
    {
        // Ignore for now
    }

    std::ranges::for_each(mTests, [&locationToWrite](ExpectedResults &tResult) {
        std::cout << "Generating test: " << tResult.test.mName << '\n';
        std::ofstream file;
        std::string fname;
        std::ranges::transform(tResult.test.mName, std::back_inserter(fname), [](auto c) {
            if (c == ' ' || c == '\t')
                return '_';
            return c;
        });

        file.open(fname);
        tResult.expected = tResult.test.generate(file, tResult.queries);
    });

    write_config_file(locationToWrite);
}
} // namespace Tests