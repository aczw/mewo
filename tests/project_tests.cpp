#include "impl/project.hpp"
#include "impl/project/pass.hpp"

#include <mewo/mewo.h>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <memory>
#include <string>

TEST_CASE("project can be created and destroyed", "[project]") {
  mewo_project_t* project = nullptr;
  mewo_result_t res = mewo_project_create(&project);

  REQUIRE(res == MEWO_SUCCESS);
  REQUIRE(project != nullptr);

  mewo_project_destroy(project);
}

TEST_CASE("set_pass_source edits one pass and bumps its revision", "[project]") {
  using mewo::impl::Pass;
  using mewo::impl::Project;

  std::unique_ptr<Project> project = Project::create();
  // Don't rely on how many passes the default project has; just make sure there are at least two.
  project->add_pass("Other", "fn main(frag_coord: vec2f) -> vec4f { return vec4f(1.0); }");

  const std::size_t pass_count = project->passes().size();
  REQUIRE(pass_count >= 2);

  const Pass::Id edited_id = project->passes().front().id();
  const Pass::Id other_id = project->passes().back().id();
  const auto edited_revision = project->passes().front().revision();
  const auto other_revision = project->passes().back().revision();
  const std::string other_source = project->passes().back().source();

  const std::string new_source = "fn main(frag_coord: vec2f) -> vec4f { return vec4f(0.0); }";

  SECTION("the edited pass gets the new source and a new revision") {
    REQUIRE(project->set_pass_source(edited_id, new_source));

    const Pass& edited = project->passes().front();
    CHECK(edited.source() == new_source);
    CHECK(edited.revision() == edited_revision + 1);
  }

  SECTION("the edited pass keeps its ID and position") {
    REQUIRE(project->set_pass_source(edited_id, new_source));

    CHECK(project->passes().size() == pass_count);
    CHECK(project->passes().front().id() == edited_id);
  }

  SECTION("other passes are untouched") {
    REQUIRE(project->set_pass_source(edited_id, new_source));

    const Pass& other = project->passes().back();
    CHECK(other.id() == other_id);
    CHECK(other.source() == other_source);
    CHECK(other.revision() == other_revision);
  }

  SECTION("every edit bumps the revision, even with identical source") {
    REQUIRE(project->set_pass_source(edited_id, new_source));
    REQUIRE(project->set_pass_source(edited_id, new_source));

    CHECK(project->passes().front().revision() == edited_revision + 2);
  }

  SECTION("an unknown ID fails and changes nothing") {
    const std::string edited_source = project->passes().front().source();

    CHECK_FALSE(project->set_pass_source(Pass::Id{0}, new_source));
    CHECK_FALSE(project->set_pass_source(Pass::Id{9999}, new_source));

    CHECK(project->passes().size() == pass_count);
    CHECK(project->passes().front().source() == edited_source);
    CHECK(project->passes().front().revision() == edited_revision);
    CHECK(project->passes().back().source() == other_source);
    CHECK(project->passes().back().revision() == other_revision);
  }
}
