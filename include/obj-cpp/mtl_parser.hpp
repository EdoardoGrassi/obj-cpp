#ifndef OBJCPP_MTL_PARSER_HPP
#define OBJCPP_MTL_PARSER_HPP

#include "obj-cpp/core.hpp"

#include <string>
#include <vector>

#if __cpp_lib_string_view
#include <string_view>
#endif

#if __cpp_lib_span
#include <span>
#endif

namespace obj
{
    /// @brief Configuration parameters for the parser.
    struct MtlParserConfig
    {
    };

    /// @brief Output produced by parsing a .mtl file.
    //template <class Value = DefaultValueType>
    struct MtlParserResult
    {
        std::vector<Material> materials;
        //std::vector<Material<Value>> materials;
    };

    /// @brief Parse the content of a file according to the .mtl format.
    [[nodiscard]] MtlParserResult parse_as_mtl(
        const std::string& s, const MtlParserConfig& c = {});

#if __cpp_lib_string_view

    /// @brief Parse the content of a file according to the .mtl format.
    [[nodiscard]] MtlParserResult parse_as_mtl(
        const std::string_view s, const MtlParserConfig& c = {});
#endif

#if __cpp_lib_span

    /// @brief Parse the content of a file according to the .mtl format.
    [[nodiscard]] MtlParserResult parse_as_mtl(
        const std::span<const char> s, const MtlParserConfig& c = {});
#endif


} // namespace obj

#endif // !OBJCPP_MTL_PARSER_HPP