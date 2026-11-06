#include <mewo/mewo.h>

#include <emscripten/html5.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define CANVAS_SELECTOR "#mewo"

typedef struct {
  mewo_runtime_t* runtime;
  mewo_surface_t* surface;
  mewo_project_t* project;
  mewo_session_t* session;

  uint32_t width, height;
  uint64_t last_pass_status_revision;
} app_t;

static void get_dimensions(uint32_t* true_width, uint32_t* true_height) {
  double width, height;
  double device_pixel_ratio = emscripten_get_device_pixel_ratio();
  emscripten_get_element_css_size("#mewo", &width, &height);

  *true_width = (uint32_t)(width * device_pixel_ratio);
  *true_height = (uint32_t)(height * device_pixel_ratio);
}

static bool on_frame(double time_ms, void* user_data) {
  app_t* app = user_data;
  mewo_runtime_t* runtime = app->runtime;

  mewo_runtime_tick(runtime);

  mewo_event_t event = {0};
  while (mewo_runtime_poll_event(runtime, &event)) {
    switch (event.type) {
      case MEWO_EVENT_DEVICE_READY: {
        printf("[info] device is ready\n");

        uint32_t width, height;
        get_dimensions(&width, &height);

        mewo_session_descriptor_t session_desc = {
          .project = app->project,
          .render_width = width,
          .render_height = height,
        };
        app->width = width;
        app->height = height;

        mewo_session_create(runtime, &session_desc, &app->session);
        app->last_pass_status_revision = 0;

        break;
      }

      default: break;
    }
  }

  if (!app->session)
    return true;

  uint32_t new_width, new_height;
  get_dimensions(&new_width, &new_height);

  if (new_width != app->width || new_height != app->height) {
    mewo_surface_resize(app->surface, new_width, new_height);
    printf("[info] resized to %u×%u\n", new_width, new_height);

    if (new_width != 0 && new_height != 0)
      mewo_session_set_render_size(app->session, new_width, new_height);

    app->width = new_width;
    app->height = new_height;
  }

  mewo_session_render(app->session, app->surface, (uint64_t)(time_ms * 1e6));

  return true;
}

int main(void) {
  static app_t app = {0};

  mewo_runtime_create(&app.runtime);

  mewo_surface_descriptor_t surface_desc = {0};
  uint32_t width, height;
  get_dimensions(&width, &height);
  surface_desc.initial_width = width;
  surface_desc.initial_height = height;
  surface_desc.type = MEWO_SURFACE_SOURCE_HTML_CANVAS;
  surface_desc.variant.html_canvas.selector = (mewo_string_view_t){
    .data = CANVAS_SELECTOR,
    .length = sizeof(CANVAS_SELECTOR),
  };
  mewo_surface_create(app.runtime, &surface_desc, &app.surface);

  mewo_project_create(&app.project);
  mewo_runtime_request_device(app.runtime, app.surface);

  emscripten_request_animation_frame_loop(on_frame, &app);
}
