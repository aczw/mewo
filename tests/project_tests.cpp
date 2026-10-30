#include "impl/project.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("mewo::test() returns 67", "[project]") { REQUIRE(mewo::impl::test() == 67); }
