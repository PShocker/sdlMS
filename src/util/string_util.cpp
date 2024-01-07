#include "string_util.hpp"
#include <locale>
#include <sstream>

namespace StringUtil
{
    void split_number(std::string &input)
    {
        for (size_t i = input.size(); i > 3; i -= 3)
            input.insert(i - 3, 1, ',');
    }

    std::string extend_id(int32_t id, size_t length)
    {
        return pad_string(std::to_string(id), length);
    }

    std::string pad_string(std::string str, size_t length)
    {
        if (str.size() < length)
            str.insert(0, length - str.size(), '0');

        return str;
    }

    std::string tolower(std::string str)
    {
        std::locale loc;
        std::stringstream ss;

        for (std::string::size_type i = 0; i < str.length(); ++i)
            ss << std::tolower(str[i], loc);

        return ss.str();
    }

    std::u16string to_ustring(std::string str)
    {
        return std::u16string{str.begin(), str.end()};
    }

    std::string to_string(std::u16string str)
    {
        return std::string{str.begin(), str.end()};
    }

    int to_int(std::u16string str)
    {
        std::string s = std::string{str.begin(), str.end()};
        return std::stoi(s);
    }
}