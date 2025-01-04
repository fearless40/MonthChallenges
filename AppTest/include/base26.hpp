#pragma once
#include <string>

namespace base26 {
std::string to(int value);  
int from( std::string_view const letters );
}