#include "core/pass.hpp"
#include "impl/project.hpp"

#include <mewo/mewo.h>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

using mewo::Pass;
using mewo::impl::Project;

constexpr const char* kName = "Pass";
constexpr const char* kSource = "fn main(frag_coord: vec2f) -> vec4f { return vec4f(1.0); }";

constexpr mewo_project_descriptor_t kDescriptor = {
  .first_pass_name = {kName, MEWO_STRING_VIEW_SENTINEL},
  .first_pass_source = {kSource, MEWO_STRING_VIEW_SENTINEL},
};

/** A project whose single pass is named `kName` with source `kSource`. */
std::unique_ptr<Project> create_project() {
  return Project::create(kName, kSource);
}

/** The project's pass IDs in list (execution) order. */
std::vector<Pass::Id> pass_ids(const Project& project) {
  std::vector<Pass::Id> ids;
  for (const Pass& pass : project.passes())
    ids.push_back(pass.id());
  return ids;
}

}  // namespace

TEST_CASE("project can be created and destroyed", "[project]") {
  mewo_project_t* project = nullptr;
  mewo_result_t res = mewo_project_create(&kDescriptor, &project);

  REQUIRE(res == MEWO_SUCCESS);
  REQUIRE(project != nullptr);

  mewo_project_destroy(project);
}

TEST_CASE("set_pass_source edits one pass and bumps its revision", "[project]") {
  using mewo::Pass;
  using mewo::impl::Project;

  std::unique_ptr<Project> project = create_project();
  // Don't rely on how many passes the default project has; just make sure there are at least two.
  project->add_pass(kName, kSource);

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
  std::unique_ptr<mewo::impl::Project> project = create_project();

  CHECK(project->common_source().empty());
  CHECK(project->common_revision() == 0);
}

TEST_CASE("set_common_source replaces the source and bumps the revision", "[project][common]") {
  using mewo::Pass;
  using mewo::impl::Project;

  std::unique_ptr<Project> project = create_project();
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
  REQUIRE(mewo_project_create(&kDescriptor, &project) == MEWO_SUCCESS);

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

TEST_CASE("add_pass appends a pass and returns its new ID", "[project][structure]") {
  std::unique_ptr<Project> project = create_project();
  const Pass::Id first = project->passes().front().id();
  const auto revision = project->structure_revision();

  const Pass::Id a = project->add_pass(kName, kSource);
  const Pass::Id b = project->add_pass(kName, kSource);

  CHECK(a != Pass::Id{0});
  CHECK(a != first);
  CHECK(b > a);
  CHECK(pass_ids(*project) == std::vector<Pass::Id>{first, a, b});
  CHECK(project->passes().back().name() == kName);
  CHECK(project->passes().back().source() == kSource);
  CHECK(project->structure_revision() == revision + 2);
}

TEST_CASE("remove_pass removes one pass and keeps the rest in order", "[project][structure]") {
  std::unique_ptr<Project> project = create_project();
  const Pass::Id first = project->passes().front().id();
  const Pass::Id a = project->add_pass(kName, kSource);
  const Pass::Id b = project->add_pass(kName, kSource);
  const auto revision = project->structure_revision();

  SECTION("removing a middle pass") {
    CHECK(project->remove_pass(a) == Project::RemovePassResult::kSuccess);

    CHECK(pass_ids(*project) == std::vector<Pass::Id>{first, b});
    CHECK(project->structure_revision() == revision + 1);
  }

  SECTION("removing the first pass") {
    CHECK(project->remove_pass(first) == Project::RemovePassResult::kSuccess);

    CHECK(pass_ids(*project) == std::vector<Pass::Id>{a, b});
  }

  SECTION("an unknown ID fails and changes nothing") {
    CHECK(project->remove_pass(Pass::Id{0}) == Project::RemovePassResult::kNotFound);
    CHECK(project->remove_pass(Pass::Id{9999}) == Project::RemovePassResult::kNotFound);

    CHECK(pass_ids(*project) == std::vector<Pass::Id>{first, a, b});
    CHECK(project->structure_revision() == revision);
  }

  SECTION("removing the same pass twice fails the second time") {
    REQUIRE(project->remove_pass(a) == Project::RemovePassResult::kSuccess);

    CHECK(project->remove_pass(a) == Project::RemovePassResult::kNotFound);
    CHECK(project->structure_revision() == revision + 1);
  }

  SECTION("IDs are never reused after a removal") {
    REQUIRE(project->remove_pass(b) == Project::RemovePassResult::kSuccess);

    const Pass::Id c = project->add_pass(kName, kSource);
    CHECK(c != b);
    CHECK(c > b);
  }
}

TEST_CASE("remove_pass refuses to remove the last pass", "[project][structure]") {
  std::unique_ptr<Project> project = create_project();
  REQUIRE(project->passes().size() == 1);
  const Pass::Id only = project->passes().front().id();
  const auto revision = project->structure_revision();

  CHECK(project->remove_pass(only) == Project::RemovePassResult::kFailedBecauseLast);

  CHECK(pass_ids(*project) == std::vector<Pass::Id>{only});
  CHECK(project->structure_revision() == revision);

  SECTION("an unknown ID is still reported as not found") {
    CHECK(project->remove_pass(Pass::Id{9999}) == Project::RemovePassResult::kNotFound);
  }

  SECTION("after removing down to one pass, that one can't be removed") {
    const Pass::Id a = project->add_pass(kName, kSource);
    REQUIRE(project->remove_pass(only) == Project::RemovePassResult::kSuccess);

    CHECK(project->remove_pass(a) == Project::RemovePassResult::kFailedBecauseLast);
    CHECK(pass_ids(*project) == std::vector<Pass::Id>{a});
  }
}

TEST_CASE("move_pass puts a pass at its new index", "[project][structure]") {
  // Five passes: p[0] .. p[4], in that order.
  std::unique_ptr<Project> project = create_project();
  std::vector<Pass::Id> p = {project->passes().front().id()};
  for (int i = 1; i < 5; ++i)
    p.push_back(project->add_pass(kName, kSource));
  REQUIRE(pass_ids(*project) == p);

  const auto revision = project->structure_revision();

  SECTION("moving right") {
    REQUIRE(project->move_pass(p[1], 3));

    CHECK(pass_ids(*project) == std::vector<Pass::Id>{p[0], p[2], p[3], p[1], p[4]});
    CHECK(project->structure_revision() == revision + 1);
  }

  SECTION("moving left") {
    REQUIRE(project->move_pass(p[3], 1));

    CHECK(pass_ids(*project) == std::vector<Pass::Id>{p[0], p[3], p[1], p[2], p[4]});
    CHECK(project->structure_revision() == revision + 1);
  }

  SECTION("moving the last pass to the front") {
    REQUIRE(project->move_pass(p[4], 0));

    CHECK(pass_ids(*project) == std::vector<Pass::Id>{p[4], p[0], p[1], p[2], p[3]});
  }

  SECTION("moving the first pass to the back") {
    REQUIRE(project->move_pass(p[0], 4));

    CHECK(pass_ids(*project) == std::vector<Pass::Id>{p[1], p[2], p[3], p[4], p[0]});
  }

  SECTION("moving to its current index succeeds without changing anything") {
    CHECK(project->move_pass(p[2], 2));

    CHECK(pass_ids(*project) == p);
    CHECK(project->structure_revision() == revision);
  }

  SECTION("an index past the end fails and changes nothing") {
    CHECK_FALSE(project->move_pass(p[0], 5));
    CHECK_FALSE(project->move_pass(p[0], UINT32_MAX));

    CHECK(pass_ids(*project) == p);
    CHECK(project->structure_revision() == revision);
  }

  SECTION("an unknown ID fails and changes nothing") {
    CHECK_FALSE(project->move_pass(Pass::Id{0}, 0));
    CHECK_FALSE(project->move_pass(Pass::Id{9999}, 0));

    CHECK(pass_ids(*project) == p);
    CHECK(project->structure_revision() == revision);
  }

  SECTION("moved passes keep their source and revision") {
    const std::string source = project->passes()[1].source();
    const auto pass_revision = project->passes()[1].revision();

    REQUIRE(project->move_pass(p[1], 4));

    CHECK(project->passes()[4].id() == p[1]);
    CHECK(project->passes()[4].source() == source);
    CHECK(project->passes()[4].revision() == pass_revision);
  }
}

TEST_CASE("editing sources doesn't change the structure revision", "[project][structure]") {
  std::unique_ptr<Project> project = create_project();
  const auto revision = project->structure_revision();

  REQUIRE(project->set_pass_source(project->passes().front().id(), kSource));
  project->set_common_source("fn helper() {}");

  CHECK(project->structure_revision() == revision);
}

TEST_CASE("mewo_project_get_pass_name returns the pass' name", "[project][c_api]") {
  mewo_project_t* project = nullptr;
  REQUIRE(mewo_project_create(&kDescriptor, &project) == MEWO_SUCCESS);

  const mewo_pass_id_t first = mewo_project_get_pass_id(project, 0);
  mewo_string_view_t name = {};

  SECTION("known ID gives the name with an explicit length") {
    REQUIRE(mewo_project_get_pass_name(project, first, &name) == MEWO_SUCCESS);

    CHECK(name.length != MEWO_STRING_VIEW_SENTINEL);
    CHECK(std::string_view(name.data, name.length) == kName);
  }

  SECTION("added passes report their own names") {
    mewo_pass_id_t added = MEWO_PASS_ID_NONE;
    REQUIRE(
      mewo_project_add_pass(
        project, {"Blur", MEWO_STRING_VIEW_SENTINEL}, {kSource, MEWO_STRING_VIEW_SENTINEL}, &added
      ) == MEWO_SUCCESS
    );

    REQUIRE(mewo_project_get_pass_name(project, added, &name) == MEWO_SUCCESS);
    CHECK(std::string_view(name.data, name.length) == "Blur");
  }

  SECTION("unknown ID is rejected and clears the out-param") {
    name = {kName, 4};

    CHECK(mewo_project_get_pass_name(project, first + 100, &name) == MEWO_ERROR_INVALID_ARGUMENT);
    CHECK(name.data == nullptr);
    CHECK(name.length == 0);
  }

  SECTION("removed pass ID is rejected") {
    mewo_pass_id_t added = MEWO_PASS_ID_NONE;
    REQUIRE(
      mewo_project_add_pass(
        project, {"Blur", MEWO_STRING_VIEW_SENTINEL}, {kSource, MEWO_STRING_VIEW_SENTINEL}, &added
      ) == MEWO_SUCCESS
    );
    REQUIRE(mewo_project_remove_pass(project, first) == MEWO_SUCCESS);

    CHECK(mewo_project_get_pass_name(project, first, &name) == MEWO_ERROR_INVALID_ARGUMENT);
  }

  mewo_project_destroy(project);
}
