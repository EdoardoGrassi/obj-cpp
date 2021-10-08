#include "obj-cpp/obj.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <sstream>
#include <string>

std::mt19937_64 rng;

[[nodiscard]] float randf(float min, float max)
{
    //return std::generate_canonical<float, 20>(rng);
    return std::uniform_real_distribution<float>{ min, max }(rng);
}

[[nodiscard]] float randf_0_1()
{
    return randf(0.f, 1.f);
}

[[nodiscard]] int randi(int min, int max)
{
    // return std::generate_canonical<int, 20>(rng);
    return std::uniform_int_distribution{ min, max }(rng);
}

[[nodiscard]] std::uint32_t randu(std::uint32_t min, std::uint32_t max)
{
    return std::uniform_int_distribution<std::uint32_t>{ min, max }(rng);
}


using namespace obj;

GTEST_TEST(Fuzzy, mtl_file_only)
{
    for (auto i = 0; i < 10; ++i)
    {
        const obj::Material material{
            .name = "",

            .ka = { randf_0_1(), randf_0_1(), randf_0_1() },
            .kd = { randf_0_1(), randf_0_1(), randf_0_1() },
            .ks = { randf_0_1(), randf_0_1(), randf_0_1() },
            .tf = { randf_0_1(), randf_0_1(), randf_0_1() },

            .illumination_model = randu(0, 10),
        };


        std::stringstream s{};
        const auto        mtl = obj::parse_as_mtl(s.view());
    }
}

GTEST_TEST(Fuzzy, obj_file_only)
{
    const auto min_v_count = 1, max_v_count = 5;
    const auto min_vn_count = 5, max_vn_count = 10;
    const auto min_vt_count = 5, max_vt_count = 10;
    const auto min_f_count = 5, max_f_count = 10;

    for (auto i = 0; i < 10; ++i)
    {
        const auto generate_vertex = []() {
            return Vertex{
                randf(-100, +100),
                randf(-100, +100),
                randf(-100, +100)
            };
        };
        std::vector<Vertex> v(randu(min_v_count, max_v_count));
        std::generate(std::begin(v), std::end(v), generate_vertex);

        const auto generate_normal = []() {
            return Normal{
                randf(-100, +100),
                randf(-100, +100),
                randf(-100, +100)
            };
        };
        std::vector<Normal> vn(randu(min_vn_count, max_vn_count));
        std::generate(std::begin(vn), std::end(vn), generate_normal);

        const auto generate_texcoord = []() {
            return Texcoord{
                randf(-100, +100),
                randf(-100, +100),
                randf(-100, +100)
            };
        };
        std::vector<Texcoord> vt(randu(min_vt_count, max_vt_count));
        std::generate(std::begin(vt), std::end(vt), generate_texcoord);

        const auto generate_face = [&]() {
            return Face{
                randu(0, static_cast<std::uint32_t>(std::size(v))),
                randu(0, static_cast<std::uint32_t>(std::size(vt))),
                randu(0, static_cast<std::uint32_t>(std::size(vn))),
                randu(0, static_cast<std::uint32_t>(std::size(v))),
                randu(0, static_cast<std::uint32_t>(std::size(vt))),
                randu(0, static_cast<std::uint32_t>(std::size(vn))),
                randu(0, static_cast<std::uint32_t>(std::size(v))),
                randu(0, static_cast<std::uint32_t>(std::size(vt))),
                randu(0, static_cast<std::uint32_t>(std::size(vn))),
            };
        };
        std::vector<Face> f(randu(min_f_count, max_f_count));
        std::generate(std::begin(f), std::end(f), generate_face);


        std::stringstream s{};
        s.precision(10);

        s << "o fuzzy-object-name\n";
        for (const auto& a : v)
            s << "v" << ' ' << a.x << ' ' << a.y << ' ' << a.z << ' ' << a.w << '\n';
        for (const auto& a : vn)
            s << "vn" << ' ' << a.x << ' ' << a.y << ' ' << a.z << '\n';
        for (const auto& a : vt)
            s << "vt" << ' ' << a.u << ' ' << a.v << ' ' << a.w << '\n';
        for (const auto& a : f)
            s << "f" << ' '
              << a.triplets[0].v << '/' << a.triplets[0].vt << '/' << a.triplets[0].vn << ' '
              << a.triplets[1].v << '/' << a.triplets[1].vt << '/' << a.triplets[1].vn << ' '
              << a.triplets[2].v << '/' << a.triplets[2].vt << '/' << a.triplets[2].vn << '\n';

        const auto obj = obj::parse_as_obj(s.view());
        EXPECT_EQ(std::size(obj.objects), 1);

        //EXPECT_EQ(obj.data.v, v);
        EXPECT_EQ(std::size(obj.data.v), std::size(v));
        for (auto i = 0; i < std::size(v); ++i)
        {
            EXPECT_FLOAT_EQ(obj.data.v[i].x, v[i].x);
            EXPECT_FLOAT_EQ(obj.data.v[i].y, v[i].y);
            EXPECT_FLOAT_EQ(obj.data.v[i].z, v[i].z);
            EXPECT_FLOAT_EQ(obj.data.v[i].w, v[i].w);
        }

        //EXPECT_EQ(obj.data.vn, vn);
        EXPECT_EQ(std::size(obj.data.vn), std::size(vn));
        for (auto i = 0; i < std::size(vn); ++i)
        {
            EXPECT_FLOAT_EQ(obj.data.vn[i].x, vn[i].x);
            EXPECT_FLOAT_EQ(obj.data.vn[i].y, vn[i].y);
            EXPECT_FLOAT_EQ(obj.data.vn[i].z, vn[i].z);
        }

        //EXPECT_EQ(obj.data.vt, vt);
        EXPECT_EQ(std::size(obj.data.vt), std::size(vt));
        for (auto i = 0; i < std::size(vt); ++i)
        {
            EXPECT_FLOAT_EQ(obj.data.vt[i].u, vt[i].u);
            EXPECT_FLOAT_EQ(obj.data.vt[i].v, vt[i].v);
            EXPECT_FLOAT_EQ(obj.data.vt[i].w, vt[i].w);
        }

        EXPECT_EQ(obj.data.faces, f);
        //EXPECT_EQ(std::size(obj.data.faces), std::size(f));
    }
}

//GTEST_TEST(Fuzzy, obj_and_mtl) {}