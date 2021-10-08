#ifndef OBJCPP_READER_HPP
#define OBJCPP_READER_HPP

#include "obj-cpp/obj.hpp"

#include <filesystem>

namespace obj
{

    template <class Value = DefaultValueType, class Index = DefaultIndexType>
    class Reader
    {
    public:

        /// @brief Load .obj file.
        [[nodiscard]] ObjParserResult load(const std::filesystem::path& p);

    private:
        std::string _buffer;
    };



} // namespace obj

#endif // !OBJCPP_READER_HPP