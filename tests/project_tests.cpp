#include <mewo/mewo.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("project can be created and destroyed", "[project]") {
  mewo_project_t* project = nullptr;
  mewo_result_t res = mewo_project_create(&project);

  REQUIRE(res == MEWO_SUCCESS);
  REQUIRE(project != nullptr);

  mewo_project_destroy(project);
}
