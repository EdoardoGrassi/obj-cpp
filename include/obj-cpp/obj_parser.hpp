#ifndef OBJCPP_OBJ_PARSER_HPP
#define OBJCPP_OBJ_PARSER_HPP

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
    struct ObjParserConfig
    {
        /// @brief Enable specific extensions to .obj standard.
        enum class ExtensionFlag
        {
            standard     = 0,
            vertex_color = (1 << 0),
        };

        /// @brief Expected number of objects.
        std::size_t expected_object_count = 1;

        /// @brief Expected number of vertices.
        std::size_t expected_vertex_count = 10'000;

        /// @brief Expected number of triangle.
        std::size_t expected_triangle_count = 10'000;

        ExtensionFlag flags = ExtensionFlag::standard;
    };


    /// @brief Output produced by parsing a .obj file.
    //template <class Value = DefaultValueType, class Index = DefaultIndexType>
    struct ObjParserResult
    {
        /// @brief Geometric data.
        MeshData data;
        //MeshData<Value> data;

        /// @brief List of objects.
        std::vector<Object> objects;
        //std::vector<Object<Index>> objects;

        // TODO: add support for groups
        /// @brief List of groups.
        //std::vector<Group<Index>> groups;
    };

    /// @brief Parse the content of a file according to the .obj format.
    ///
    /// @param[in] s Source text to parse.
    /// @param[in] c Parser configuration.
    ///
    /// @return Parsed content.
    //template <class Value = DefaultValueType, class Index = DefaultIndexType>
    [[nodiscard]] ObjParserResult parse_as_obj(
        const std::string& s, const ObjParserConfig& c = {});

#if __cpp_lib_string_view

    /// @brief Parse the content of a file according to the .obj format.
    [[nodiscard]] ObjParserResult parse_as_obj(
        const std::string_view s, const ObjParserConfig& c = {});
#endif

#if __cpp_lib_span

    /// @brief Parse the content of a file according to the .obj format.
    [[nodiscard]] ObjParserResult parse_as_obj(
        const std::span<const char> s, const ObjParserConfig& c = {});
#endif

} // namespace obj

#endif // !OBJCPP_OBJ_PARSER_HPP