#include "exception.hpp"
#include "mewo.hpp"

#include <cstdlib>
#include <exception>
#include <print>

int main(int, char*[])
{
  try {
    mewo::run();
  } catch (const mewo::Exception& ex) {
    std::println("Unhandled Mewo exception. {}", ex.what());
    return EXIT_FAILURE;
  } catch (const std::exception& ex) {
    std::println("Unhandled system exception. {}", ex.what());
    return EXIT_FAILURE;
  } catch (...) {
    std::println("Unknown exception occurred");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
