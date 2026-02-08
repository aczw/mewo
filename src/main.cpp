#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <print>

namespace {

constexpr auto TIMED_WAIT_ANY = wgpu::InstanceFeatureName::TimedWaitAny;
constexpr auto SDL_INIT_FLAGS = SDL_INIT_VIDEO;
constexpr int SCREEN_W = 640;
constexpr int SCREEN_H = 480;

}

int main(int, char*[])
{
  if (!SDL_Init(SDL_INIT_FLAGS)) {
    std::println(stderr, "Failed to initialize SDL: {}", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_WindowFlags window_flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
  SDL_Window* window = SDL_CreateWindow("Mewo 0.0.1-alpha", SCREEN_W, SCREEN_H, window_flags);

  if (!window) {
    std::println(stderr, "Failed to create SDL window: {}", SDL_GetError());
    return EXIT_FAILURE;
  }

  wgpu::InstanceDescriptor instance_desc = {
    .requiredFeatureCount = 1,
    .requiredFeatures = &TIMED_WAIT_ANY,
  };

  wgpu::Instance instance = wgpu::CreateInstance(&instance_desc);

  if (instance == nullptr) {
    std::println(stderr, "Instance creation failed");
    return EXIT_FAILURE;
  }

  wgpu::RequestAdapterOptions options = {};
  wgpu::Adapter adapter = {};

  auto callback = [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, const char* message,
                      void* user_data) {
    if (status != wgpu::RequestAdapterStatus::Success) {
      std::println(stderr, "Failed to get an adapter: {}", message);
      return;
    }

    *static_cast<wgpu::Adapter*>(user_data) = adapter;
  };
  auto callback_mode = wgpu::CallbackMode::WaitAnyOnly;

  void* user_data = &adapter;
  instance.WaitAny(instance.RequestAdapter(&options, callback_mode, callback, user_data),
      std::numeric_limits<uint64_t>::max());

  if (adapter == nullptr) {
    std::println(stderr, "RequestAdapter failed");
    return EXIT_FAILURE;
  }

  wgpu::DawnAdapterPropertiesPowerPreference power_props = {};
  wgpu::AdapterInfo info = {};
  info.nextInChain = &power_props;

  adapter.GetInfo(&info);
  std::println("Hello Mewo from C++ {}", __cplusplus);
  std::println("VendorID: {:#x}", info.vendorID);
  std::println("Vendor: {}", info.vendor.data);
  std::println("Architecture: {}", info.architecture.data);
  std::println("DeviceID: {:#x}", info.deviceID);
  std::println("Name: {}", info.device.data);
  std::println("Driver description: {}", info.description.data);

  bool will_quit = false;
  SDL_Event event = {};

  while (!will_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        will_quit = true;
      }
    }
  }

  SDL_DestroyWindow(window);
  SDL_QuitSubSystem(SDL_INIT_FLAGS);
  SDL_Quit();

  return EXIT_SUCCESS;
}
