
#include "generatetest.hpp"
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <ranges>
#include <sstream>
#include <string_view>
#include <variant>

using std::operator""sv;


std::random_device randomDevice;
std::mt19937 randomGen(randomDevice());

bool bool_rand() {
    std::uniform_int_distribution<> r(0,1);
    return r(randomGen) == 0 ? false : true;
}

template<typename T>
auto between_rand( T min, T max) -> T{
    std::uniform_int_distribution<T> r(min,max);
    return r(randomGen);
}


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



struct RowCol {
    std::uint16_t row;
    std::uint16_t col;

    std::string excel() {
        const std::string_view letters {"abcdefghijklmnopqrstuvwxyz"}; 


    }

    std::string colrow() {
        return std::format("{},{}", col,row);
    }

    static RowCol random( std::uint16_t maxRow, std::uint16_t maxCol ) {
        return { between_rand<std::uint16_t>( 0, maxRow),
                 between_rand<std::uint16_t>(0, maxCol) 
                };
    }

    bool operator == ( const RowCol & other) const = default;
};

struct QueryErrorAnswer {
    bool is_error; 
};

struct QueryCorrectAnswer {
    RowCol pos;
    std::int16_t answer;
};

using QueryAnswer = std::variant<QueryCorrectAnswer, QueryErrorAnswer>;
using Queries = std::vector<RowCol>;
using QueryAnswers = std::vector<QueryAnswer>;

class TestDefinition
{
    public:

       /* constexpr TestDefinition(const std::string_view name, std::uint16_t nbrRows, std::uint16_t nbrCols, RowColData dataGenerateFun, Errors errors, bool injectWhiteSpace ) :
            mName(name) ,mNbrRows( nbrRows), mNbrCols(nbrCols), mData(dataGenerateFun), mError(errors), mInjectRandomWhiteSpace(injectWhiteSpace) {

            }
*/

        std::string_view mName;
        const std::uint16_t mNbrRows;
        const std::uint16_t mNbrCols;
        const RowColData mData{RowColData::IncrementFromPos};
        const Errors mError{Errors::None};
        const bool mInjectRandomWhiteSpace{false};

        bool huge() const {
            return  mNbrRows == std::numeric_limits<std::uint16_t>::max() &&
                    mNbrCols == std::numeric_limits<std::uint16_t>::max();
        }
        
       QueryAnswers generate( std::ostream & file, const std::vector<RowCol> & guesses  ) const {
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> bool_random(0,1);
            std::uniform_int_distribution<> count_random(1,4); 

        
            auto injectWhiteSpace = [&](std::size_t count = 1){ 
                char ws = (bool_random(gen) == 0 ? '\t' : ' ');
                for( int i=0; i < count; ++i)
                    file <<  ws;
            };      


            if( mInjectRandomWhiteSpace )
                injectWhiteSpace( count_random(gen) );
                
            file << make_row_value(mNbrRows, mError) << '\n';

            if( mInjectRandomWhiteSpace )
                injectWhiteSpace( count_random(gen) );

            file << make_col_value(mNbrCols, mError) << '\n';

            return write_data_value(file, guesses);
        };
    
    private:

    std::string make_row_value(std::size_t value, Errors err) const
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

    std::string make_col_value(std::size_t value, Errors err) const 
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

    QueryAnswers write_data_value(std::ostream &file, const std::vector<RowCol> & guesses) const
    {
        if (!file)
            return {};

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::int16_t> dataRandom(std::numeric_limits<std::int16_t>::min(),
                                                std::numeric_limits<std::int16_t>::max());
        std::uniform_int_distribution<> boolGenerator(0,1);
        std::uniform_int_distribution<std::size_t> random_row(0, mNbrRows);
        std::uniform_int_distribution<std::size_t> random_col(0, mNbrCols);

        auto bool_random = [&boolGenerator, &gen]() -> bool {
            if( boolGenerator( gen ) == 0)
                return false;
            return true;
        };

        auto row_random = [&random_row, &gen]() {
            return random_row(gen);
        };

        auto col_random = [&random_col, &gen]() {
            return random_col(gen);
        };

        auto increment_values = [](std::int16_t lastValue) -> std::int16_t {
            if (lastValue == std::numeric_limits<std::int16_t>::max())
                return 0;
            return ++lastValue;
        };

        auto random_values = [&]() -> std::int16_t { return dataRandom(gen); };

        auto injectWhiteSpace = [&](std::size_t count = 1){ 
            
            char ws = (bool_random() == true ? '\t' : ' ');
            for( int i=0; i < count; ++i)
                file <<  ws;
        };          
        
        std::int16_t lastValue = (mData == RowColData::Random) || mData == RowColData::IncrementFromPos
                                    ? 0
                                    : std::numeric_limits<std::int16_t>::min();

        const std::size_t rowToGen = mError == Errors::DataMissingRow ? mNbrRows - 1 : mNbrRows;
        bool skipOneColumn = mError == Errors::DataMissingCol ? true : false;
        std::size_t randCol = col_random();
        std::size_t randRow = row_random();

        QueryAnswers answers; 
        answers.reserve( guesses.size() );
        
        for (std::size_t row = 0; row < rowToGen; ++row)
        {
            for (std::size_t col = 0; col < mNbrCols; ++col)
            {
                if( skipOneColumn && randCol == col)
                {
                    skipOneColumn = false;
                    continue;
                }

                if( col != 0) file << ',';
                
                lastValue = mData == RowColData::Random ? random_values() : increment_values(lastValue);

                if( mInjectRandomWhiteSpace && bool_random())
                        injectWhiteSpace(row_random()); 

                if( row == randRow && mError != Errors::None && col == randCol) {
                    if( mError == Errors::DataValueTooLarge ) 
                        file << std::numeric_limits<std::int16_t>::max() + random_values();
                    else if( mError == Errors::DataValueTooSmall )
                        file << std::numeric_limits<std::int16_t>::min() - random_values();
                    else if( mError == Errors::DataHasText )
                        file << "ab1234df";
                }
                else {
                    if( mError == Errors::None) {
                        RowCol current {static_cast<std::uint16_t>(row), static_cast<std::uint16_t>(col)};
                        auto it = std::ranges::find( guesses , current ); 
                        if( it != guesses.end() ) {
                            QueryCorrectAnswer q{ current, lastValue };
                            answers.emplace_back( q );
                        }
                    }
                    file << lastValue;
                }
                    
                    
                if( mInjectRandomWhiteSpace && bool_random())
                    injectWhiteSpace(row_random());
            }
            file << '\n';
        }

        if( mError != Errors::None) {
            return {};
        }
        return answers;
    }
    

};


class TestsConfiguration
{
    private:
    struct TestResults {
        const TestDefinition & test; 
        Queries              queries; 
        QueryAnswers         expected; 
        std::vector<std::string> rejected;
    };

    std::vector<TestResults> mTests; 

    public:
    void create( const TestDefinition & test, std::size_t nbrQueries,  std::size_t outofbounds  ) {
        TestResults t{test};
        
        if( test.mError != Errors::None ) {
            t.expected.emplace_back(QueryErrorAnswer{true});
            mTests.emplace_back( t );
            return;
        }

        // If there are no errors ensure that a report of error is rejected
        t.rejected.emplace_back("error");

        // Generate queries
        for( int queryCount = 0; queryCount < nbrQueries; ++queryCount ){
            t.queries.emplace_back( between_rand<std::uint16_t>( 0, test.mNbrRows),
                                    between_rand<std::uint16_t>( 0, test.mNbrCols));
        }

        // Generate Out of bound queries
        for( std::size_t oobCount = 0; oobCount < outofbounds; ++oobCount){
            t.queries.emplace_back( between_rand<std::uint16_t>( test.mNbrRows + 1, test.mNbrRows * 30),
                                    between_rand<std::uint16_t>( test.mNbrCols + 1, test.mNbrCols * 30));
        }

        mTests.emplace_back( t );
    }

    void write_all_tests( std::filesystem::path locationToWrite, bool locateTestFileInSeperateFolder ) {
        if( locateTestFileInSeperateFolder ) {
            // Ignore for now
        }

        std::ranges::for_each( mTests, [&locationToWrite]( TestResults & tResult) {
           std::cout << "Generating test: " << tResult.test.mName << '\n';
           std::ofstream file;
           std::string fname; 
           std::ranges::transform(tResult.test.mName, std::back_inserter(fname), [](auto c) {
            if (c == ' ' || c == '\t')
                return '_';
            return c;
            });    

            file.open( fname );
            tResult.expected = tResult.test.generate( file, tResult.queries);



        });

        write_config_file( locationToWrite );
    }

    private: 
        void write_config_file( std::filesystem::path locationToWrite ){
            std::cout << "Writing Config file" << '\n';
        }
    
};

constexpr TestDefinition mErr(Errors err, const std::string_view name)
{
    return {name, 8, 8, RowColData::IncrementFromPos, err};
}

const TestDefinition tests[] = {
    {"Small Test", 2, 2},
    {"Medium Test", 8, 8},
    {"Large Test", 30, 30},
    {"Row Col Diff", 2,8},
    {"Col Row Diff", 8,2},
    {"Small Test - Negative numbers", 2, 2, RowColData::IncrementFromNeg},
    {"Medium Test - Negative numbers", 8, 8, RowColData::IncrementFromNeg},
    {"Large Test  - Negative numbers", 30, 30, RowColData::IncrementFromNeg},
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


int generate_tests_cmd_line(std::filesystem::path test_output, CommandLine::TestModes mode, bool huge, bool overwrite)
{
    auto currentPath = std::filesystem::current_path();
    std::cout << "Current Path: " << currentPath << '\n';
    TestsConfiguration config;

    std::ranges::for_each(
    tests | std::views::filter(
        [&mode](auto &t){
                if( mode == CommandLine::TestModes::NoErrors) 
                    return t.mError == Errors::None;
                return true;
         }
    ),
    [&config, &currentPath](const TestDefinition &test)
    { 
        config.create( test, 5, 2);
        //make_test(config, test, currentPath); 
    });

    config.write_all_tests( std::filesystem::current_path(), false );

    return 0;
}
