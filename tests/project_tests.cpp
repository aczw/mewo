#include "core/pass.hpp"
#include "impl/project.hpp"

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
  using mewo::Pass;
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

TEST_CASE("common source starts empty at revision 0", "[project][common]") {
  std::unique_ptr<mewo::impl::Project> project = mewo::impl::Project::create();

  CHECK(project->common_source().empty());
  CHECK(project->common_revision() == 0);
}

TEST_CASE("set_common_source replaces the source and bumps the revision", "[project][common]") {
  using mewo::Pass;
  using mewo::impl::Project;

  std::unique_ptr<Project> project = Project::create();
  const std::string common = "fn helper() -> f32 { return 1.0; }";

  SECTION("the new source is stored and the revision moves") {
    const auto revision = project->common_revision();
    project->set_common_source(common);

    CHECK(project->common_source() == common);
    CHECK(project->common_revision() == revision + 1);
  }

  SECTION("every edit bumps the revision, even with identical source") {
    const auto revision = project->common_revision();
    project->set_common_source(common);
    project->set_common_source(common);

    CHECK(project->common_revision() == revision + 2);
  }

  SECTION("setting it back to empty is an edit too") {
    project->set_common_source(common);
    const auto revision = project->common_revision();
    project->set_common_source("");

    CHECK(project->common_source().empty());
    CHECK(project->common_revision() == revision + 1);
  }

  SECTION("passes are untouched") {
    const Pass::Id id = project->passes().front().id();
    const std::string source = project->passes().front().source();
    const auto revision = project->passes().front().revision();

    project->set_common_source(common);

    CHECK(project->passes().front().id() == id);
    CHECK(project->passes().front().source() == source);
    CHECK(project->passes().front().revision() == revision);
  }

  SECTION("pass edits don't move the common revision") {
    project->set_common_source(common);
    const auto revision = project->common_revision();

    REQUIRE(project->set_pass_source(project->passes().front().id(), "fn main() {}"));

    CHECK(project->common_source() == common);
    CHECK(project->common_revision() == revision);
  }
}

TEST_CASE("mewo_project_set_common_source converts the C string", "[project][common][c_api]") {
  mewo_project_t* project = nullptr;
  REQUIRE(mewo_project_create(&project) == MEWO_SUCCESS);

  const mewo::impl::Project& impl = *project->impl;
  const auto revision = impl.common_revision();

  SECTION("explicit length uses exactly that many bytes") {
    const char text[] = "fn a() {}EXTRA";
    REQUIRE(mewo_project_set_common_source(project, {text, 9}) == MEWO_SUCCESS);

    CHECK(impl.common_source() == "fn a() {}");
    CHECK(impl.common_revision() == revision + 1);
  }

  SECTION("sentinel length reads up to the NUL") {
    REQUIRE(
      mewo_project_set_common_source(project, {"fn b() {}", MEWO_STRING_VIEW_SENTINEL}) ==
      MEWO_SUCCESS
    );

    CHECK(impl.common_source() == "fn b() {}");
  }

  SECTION("null data with zero length is an empty source") {
    REQUIRE(mewo_project_set_common_source(project, {nullptr, 0}) == MEWO_SUCCESS);

    CHECK(impl.common_source().empty());
    CHECK(impl.common_revision() == revision + 1);
  }

  SECTION("null data with a non-zero length is rejected and changes nothing") {
    CHECK(mewo_project_set_common_source(project, {nullptr, 4}) == MEWO_ERROR_INVALID_ARGUMENT);

    CHECK(impl.common_source().empty());
    CHECK(impl.common_revision() == revision);
  }

  mewo_project_destroy(project);
}
