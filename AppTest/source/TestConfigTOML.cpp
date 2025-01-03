#include "TestConfigTOML.hpp"
#include "toml++/toml.hpp"
#include <iostream>

namespace Tests
{

const std::string_view Config_File_Version = "1.0";

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

Queries parse_queries(toml::array const & queries)
{
    Queries q;
    queries.for_each( [&](auto && value) {
        if constexpr  (toml::is_string<decltype(value)>)
        {
            RowCol rc = RowCol::from_string( *value );
            q.push_back( rc );
        }
        
    });
    return q;
}

std::vector<std::string> parse_rejected( toml::array const & arr) {
    std::vector<std::string> data;
    
    arr.for_each( [&]( auto && value) {
        if constexpr (toml::is_string<decltype(value)>)
            data.push_back( *value );
    });
    return data;
}

QueryAnswers parse_query_answers( toml::array const & arr ){
    QueryAnswers qa; 
    for( auto const & it : arr) {
        if( it.is_table() ){
            QueryAnswer q{};
            auto const & tb = *it.as_table();
            auto answer_node = tb["answer"];
            if (answer_node.is_string()) {
                q.answer = 0;
                q.is_oob = true;
            } else if (answer_node.is_integer()) {
                q.answer = answer_node.value_or<std::int16_t>(0);
                q.is_oob = false;
            }

            auto queryNode = tb["query"];
            if( queryNode ){
                q.pos = RowCol::from_string( queryNode.value_or<std::string_view>("0,0"));
            }

            qa.push_back(q);
        }
    }
    return qa;
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

        toml::table const &table = *(it->as_table());
        Definition t{ };

        std::string filename; 

        filename    = table["filename"].value_or<std::string>(""); 
        t.mName     = table["testName"].value_or<std::string>("");
        t.mNbrRows  = table["rowCount"].value_or<std::uint16_t>(0);
        t.mNbrCols  = table["colCount"].value_or<std::uint16_t>(0);
        t.mError   = table["errorCode"].value_or<Errors>(Errors::None);
        t.mInjectRandomWhiteSpace = table["hasRandomWhiteSpace"].value_or<bool>(false);
        std::vector<std::string> rejected; 
        QueryAnswers answers;
        Queries queries;


        {
            // Parse query
            auto temp = table["queries"].as_array();
            if(temp) {
                queries = parse_queries( *temp );
            }
        }

        {
            // Parse rejected
            auto temp = table["rejected"].as_array();
            if( temp ){
                rejected = parse_rejected( *temp );
            }
        }

        {
            auto temp = table["expected"].as_array();
            if( temp ) {
                answers = parse_query_answers( *temp );
            }
        }

     
        config.add_existing( t, std::move(filename), std::move(queries), std::move(answers), std::move(rejected));
        
    }

    return config;
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

} // namespace Tests