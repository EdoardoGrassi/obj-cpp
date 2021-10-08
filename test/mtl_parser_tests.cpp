#include "obj-cpp/mtl_parser.hpp"

#include <gtest/gtest.h>

using namespace obj;

GTEST_TEST(MtlParser, SingleMaterial)
{
    const std::string source = "newmtl my_custom_material\n";

    const std::vector<Material> expected = {
        { .name = "my_custom_material" },
    };

    const auto r = obj::parse_as_mtl(source);
    ASSERT_EQ(r.materials, expected);
}

GTEST_TEST(MtlParser, MultiMaterial)
{
    const std::string source = "newmtl my_custom_material_001\n"
                               "newmtl my_custom_material_002\n"
                               "newmtl my_custom_material_003\n";

    const std::vector<Material> expected = {
        { .name = "my_custom_material_001" },
        { .name = "my_custom_material_002" },
        { .name = "my_custom_material_003" },
    };

    const auto r = obj::parse_as_mtl(source);
    ASSERT_EQ(r.materials, expected);
}