#include "base26.hpp"
#include <string>
#include <ranges>   
#include <cmath>
#include <algorithm>  

namespace base26{

constexpr std::size_t reserve_amount( int value ) {
    if (value < 26) 
        return 1;
    if( value >=26 && value <= 9999999)
        return 6;

    return 8;
}

std::string to(int value)  
{
    if (value == 0)
    {
        return "a";
    }
    
    std::string result{};
    result.reserve(reserve_amount( value ));
        
    while (value > 0)
    {
        auto out = std::div(value, 26);
        result += static_cast<char>('a' + out.rem);
        value = out.quot;
    }
    
    std::ranges::reverse(result);
    return result;
}

char toLower (char l) { return std::tolower(static_cast<unsigned char>(l)); };

int from( std::string_view const letters ){

    std::size_t numberLetters = letters.size();
    int value = 0;
    for (auto l : letters | std::views::transform(toLower))
    {
        value += (l - 'a') * static_cast<int>(std::pow(26, numberLetters - 1));
        --numberLetters;
    }
    return value;
}
}