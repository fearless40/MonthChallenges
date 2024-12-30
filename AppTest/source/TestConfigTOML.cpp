#include "TestConfigTOML.hpp"
#include "toml++/toml.hpp"
#include <iostream>

namespace Tests
{

const std::string_view Config_File_Version = "version";

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
    root.insert("version", Config_File_Version);

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

Configuration parse_tests(toml::array const *arr)
{
    Configuration config;
    for (auto it = arr->begin(); it != arr->end(); ++it)
    {

        if (!it->is_table())
        {
            return {};
        }

        auto const &table = it->as_table();
        std::string filename;
        std::string testname;
        std::uint16_t rowCount{0};
        std::uint16_t colCount{0};
        Errors errorCode{Errors::None};
        bool hasError{false};
        bool huge{false};
        bool hasRandomWhiteSpace{false};

        table->visit([&](const toml::key &key, auto &&node) {
            if (key == "filename")
            {
                filename = node.as_string();
            }
            else if (key == "testname")
            {
                testname = node.as_string();
            }
            else if (key == "rowCount")
            {
                rowCount = static_cast<std::uint16_t>(node.as_integer());
            }
            else if (key == "colCount")
            {
                colCount = static_cast<std::uint16_t>(node.as_integer());
            }
            else if (key == "errorCode")
            {
                errorCode = static_cast<Errors>(node.as_integer());
            }
            else if (key == "hasError")
            {
                hasError = node.as_boolean();
            }
            else if (key == "isHuge")
            {
                huge = node.as_boolean();
            }
            else if (key == "hasRandomWhiteSpace")
            {
                hasRandomWhiteSpace = node.as_boolean();
            }
            else if (key == "queries")
            {
                // Load queries
            }
        })

            auto node = table["filename"];
    }
}

Configuration toml_file_to_config(std::filesystem::path filename)
{

    toml::parse_result result;
    try
    {
        result = toml::parse_file(filename.c_str());
    }
    catch (toml::parse_error err)
    {
        std::cout << "Parse error of config file: " << err.description() << '\n';
        return Configuration{};
    }

    if (result["version"] != Config_File_Version)
    {
        std::cout << "Invalid file configuration version." << '\n';
        return Configuration{};
    }

    auto allTests = result["tests"];

    if (allTests.is_array_of_tables())
    {
        return parse_tests(allTests.as_array());
    }

    return {};
}
}
} // namespace Tests