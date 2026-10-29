#include "project_impl.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("mewo::test() returns 67", "[project]") { REQUIRE(mewo::test() == 67); }
