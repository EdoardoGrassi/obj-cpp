#pragma once
#ifndef OBJCPP_CORE_HPP
#define OBJCPP_CORE_HPP

#include <array>
#include <string>
#include <vector>

namespace obj
{
    using Value = float;
    using Index = std::int32_t;

    /// @brief Geometric vertex.
    struct alignas(16) Vertex
    {
        explicit constexpr Vertex(Value x, Value y, Value z, Value w) noexcept
            : x{ x }, y{ y }, z{ z }, w{ w } {}

        /// @brief X component.
        Value x;

        /// @brief The y component.
        Value y;

        /// @brief The z component.
        Value z;

        /// @brief Vertex weight of the vector.
        Value w;

        [[nodiscard]] bool operator==(const Vertex&) const = default;
        [[nodiscard]] bool operator!=(const Vertex&) const = default;
    };

    struct alignas(16) Normal
    {
        explicit constexpr Normal(Value x, Value y, Value z) noexcept
            : x{ x }, y{ y }, z{ z } {}

        Value x;
        Value y;
        Value z;

        [[nodiscard]] bool operator==(const Normal&) const = default;
        [[nodiscard]] bool operator!=(const Normal&) const = default;
    };

    struct alignas(16) Texcoord
    {
        explicit constexpr Texcoord(Value u, Value v, Value w) noexcept
            : u{ u }, v{ v }, w{ w } {}

        Value u;
        Value v = 0.f;
        Value w = 0.f;

        [[nodiscard]] bool operator==(const Texcoord&) const = default;
        [[nodiscard]] bool operator!=(const Texcoord&) const = default;
    };


    struct Triplet
    {
        Index v, vt, vn;
    };


    struct Face
    {
        //std::array<Tndex, 3> v;
        //std::array<Index, 3> vn;
        //std::array<Index, 3> vt;
        std::array<Triplet, 3> triplets;
    };

    /// @brief Mesh data from a whole .obj file.
    struct MeshData
    {
        /// @brief List of geometry vertices ('v' statements).
        std::vector<Vertex> v;

        /// @brief List of normal vectors ('vn' statement).
        std::vector<Normal> vn;

        /// @brief List of texture vertices ('vt' statements).
        std::vector<Texcoord> vt;

        /// @brief List of face elements ('f' statement).
        std::vector<Face> faces;
    };

    /// @brief Group of element under the same group tag.
    struct Group
    {
        std::string        name;
        std::vector<Index> faces;
    };

    /// @brief Group of elements under the same object tag.
    struct Object
    {
        std::string        name;
        std::vector<Index> faces;
    };


} // namespace obj

#endif // !OBJCPP_CORE_HPP