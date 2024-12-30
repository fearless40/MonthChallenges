#include "TestConfigTOML.hpp"
#include "toml++/toml.hpp"

namespace Tests
{

void insert_test(toml::array &root, const Configuration::ExpectedResults &result)
{
    toml::table r{};

    r.insert("filename", result.filename);
    r.insert("testName", result.test.mName);
    r.insert("rowCount", result.test.mNbrRows);
    r.insert("colCount", result.test.mNbrCols);
    r.insert("errorCode", result.test.mError);
    r.insert("hasError", result.test.mError != Errors::None);
    r.insert("isHuge", result.test.huge());
    r.insert("hasRandomWhiteSpace", result.test.mInjectRandomWhiteSpace);

    toml::array queries{};
    for (const auto &q : result.queries)
    {
        queries.push_back(q.as_colrow_fmt());
    }

    r.insert("queries", queries);

    toml::array rejected{};
    for (const auto &r : result.rejected)
    {
        rejected.push_back(r);
    }

    r.insert("rejected", rejected);

    toml::array expected{};

    for (const auto &exp : result.expected)
    {
        toml::table tb;

        if (exp.is_oob)
            tb.insert("answer", "OOB");
        else
            tb.insert("answer", exp.answer);

        tb.insert("query", exp.pos.as_colrow_fmt());

        expected.push_back(tb);
    }

    r.insert("expected", expected);

    root.push_back(r);
}

bool config_to_toml_file(const Configuration &config, std::filesystem::path filename)
{
    toml::table root;
    toml::array tests;
    root.insert("version", "1.0");

    for (const auto &test : config)
    {
        insert_test(tests, test);
    }

    root.insert("tests", tests);

    std::ofstream file;
    file.open(filename);
    file << root;
    file.close();
    return true;
}
} // namespace Tests