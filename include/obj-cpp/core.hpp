#pragma once
#ifndef OBJCPP_CORE_HPP
#define OBJCPP_CORE_HPP

#include <array>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

namespace obj
{
    using Value = float;
    using Index = std::size_t;

    /// @brief Default type in template parameters for real values.
    using DefaultValueType = float;

    /// @brief Default type in template parameters for index values.
    using DefaultIndexType = std::int32_t;


    /// @brief Geometric vertex.
    //template <class Value = DefaultValueType>
    struct alignas(16) Vertex
    {
        /// @brief The x component.
        Value x;

        /// @brief The y component.
        Value y;

        /// @brief The z component.
        Value z;

        /// @brief Vertex weight of the vector (optional).
        Value w;

        [[nodiscard]] constexpr bool operator==(const Vertex&) const noexcept = default;
        [[nodiscard]] constexpr bool operator!=(const Vertex&) const noexcept = default;
    };

    /// @brief Geometric normal.
    //template <class Value = DefaultValueType>
    struct alignas(16) Normal
    {
        /// @brief The x component.
        Value x;

        /// @brief The y component.
        Value y;

        /// @brief The z component.
        Value z;

        [[nodiscard]] constexpr bool operator==(const Normal&) const noexcept = default;
        [[nodiscard]] constexpr bool operator!=(const Normal&) const noexcept = default;
    };

    /// @brief Geometric texture coordinates.
    //template <class Value = DefaultValueType>
    struct alignas(16) Texcoord
    {
        /// @brief First coordinate component u.
        Value u;

        /// @brief Second coordinate component v (optional).
        Value v = 0.f;

        /// @brief Third coordinate component w (optional).
        Value w = 0.f;

        [[nodiscard]] constexpr bool operator==(const Texcoord&) const noexcept = default;
        [[nodiscard]] constexpr bool operator!=(const Texcoord&) const noexcept = default;
    };


    /// @brief Gemotric vertex, normal and texture coordinate triple.
    //template <class Index = DefaultIndexType>
    struct Triplet
    {
        /// @brief Zero-based index of a vertex.
        Index v;

        /// @brief Zero-based index of a normal.
        Index vt;

        /// @brief Zero-based index of a texture coordinate.
        Index vn;

        [[nodiscard]] constexpr bool operator==(const Triplet&) const noexcept = default;
        [[nodiscard]] constexpr bool operator!=(const Triplet&) const noexcept = default;
    };


    /// @brief Triangular face.
    //template <class Index = DefaultIndexType>
    struct Face
    {
        std::array<Triplet, 3> triplets;
        //std::array<Triplet<Index>, 3> triplets;

        [[nodiscard]] constexpr bool operator==(const Face&) const noexcept = default;
        [[nodiscard]] constexpr bool operator!=(const Face&) const noexcept = default;
    };


    /// @brief A range of indices defined as [begin, end).
    //template <class Index = DefaultIndexType>
    struct IndexRange
    {
        Index begin;
        Index end;

        [[nodiscard]] constexpr bool operator==(const IndexRange&) const noexcept = default;
        [[nodiscard]] constexpr bool operator!=(const IndexRange&) const noexcept = default;
    };


    /// @brief Polygonal data covered by the scope of a statement.
    //template <class Index = DefaultIndexType>
    struct PolygonalDataScope
    {
        /// @brief Range of vertex indices included in the scope.
        IndexRange vertices;
        //IndexRange<Index> vertices;

        /// @brief Range of normals indices included in the scope.
        IndexRange normals;
        //IndexRange<Index> normals;

        /// @brief Range of texture coordinate indices included in the scope.
        IndexRange texcoords;
        //IndexRange<Index> texcoords;

        /// @brief Range of faces indices included in the scope.
        IndexRange faces;
        //IndexRange<Index> faces;
    };


    /// @brief Free-form data covered by the scope of a statement.
    //template <class Index = DefaultIndexType>
    struct FreeformDataScope
    {
        // TODO: add support for free-form data
    };


    /// @brief Mesh data from a whole .obj file.
    //template <class Value = DefaultValueType, class Index = DefaultIndexType>
    struct MeshData
    {
        /// @brief List of geometry vertices ('v' statements).
        std::vector<Vertex> v;
        //std::vector<Vertex<Value>> v;

        /// @brief List of normal vectors ('vn' statement).
        std::vector<Normal> vn;
        //std::vector<Normal<Value>> vn;

        /// @brief List of texture vertices ('vt' statements).
        std::vector<Texcoord> vt;
        //std::vector<Texcoord<Value>> vt;

        /// @brief List of face elements ('f' statement).
        std::vector<Face> faces;
        //std::vector<Face<Index>> faces;
    };


    /// @brief Group of element under the same group tag.
    //template <class Index = DefaultIndexType>
    struct Group
    {
        /// @brief Group name.
        std::string name;

        /// @brief List of faces associated with the object.
        std::vector<Index> faces;
    };


    /// @brief Group of elements under the same object tag.
    //template <class Index = DefaultIndexType>
    struct Object
    {
        /// @brief Object name.
        std::string name;

        /// @brief List of faces associated with the object.
        std::vector<Index> faces;
    };


    /// @brief Material definition from a .mtl file.
    //template <class Value = DefaultValueType>
    struct Material
    {
        /// @brief Associated material name.
        std::string name;

        /// @brief Ambient reflectivity.
        Value ka[3] = { 0.2f, 0.2f, 0.2f };

        /// @brief Diffuse reflectivity.
        Value kd[3] = { 0.8f, 0.8f, 0.8f };

        /// @brief Specular reflectivity.
        Value ks[3] = { 1.0f, 1.0f, 1.0f };

        /// @brief Transmission filter.
        Value tf[3] = { 0.0f, 0.0f, 0.0f };

        /// @brief Index of the illumination model.
        std::uint32_t illumination_model;

        std::string map_ka;
        std::string map_kd;
        std::string map_ks;

#if defined(OBJCPP_PBR_EXT)

#endif
        [[nodiscard]] constexpr bool operator==(const Material&) const noexcept = default;
        [[nodiscard]] constexpr bool operator!=(const Material&) const noexcept = default;
    };


} // namespace obj

#endif // !OBJCPP_CORE_HPP