#include <iostream>
#include <unordered_map>
#include <cstdint>

namespace util
{

    namespace StringUtil
    {
        // Format a number string so that each three decimal points are separated by a comma
        void split_number(std::string &input);

        // Prefix an id with zeros so that it has the minimum specified length
        std::string extend_id(int32_t id, size_t length);

        // Pad a string with leading zeros to a specified length
        std::string pad_string(std::string str, size_t length);

        // Convert a string to lowercase letters
        std::string tolower(std::string str);

        std::u16string to_ustring(std::string str);

        std::string to_string(std::u16string str);
    }
}