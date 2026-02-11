#include "exception.hpp"
#include "mewo.hpp"

#include <cstdlib>
#include <exception>
#include <print>

int main(int, char*[])
{
  int status = EXIT_SUCCESS;

  try {
    mewo::Mewo mewo;
    mewo.run();
  } catch (const mewo::Exception& ex) {
    std::println("Unhandled Mewo exception. {}", ex.what());
    status = EXIT_FAILURE;
  } catch (const std::exception& ex) {
    std::println("Unhandled system exception. {}", ex.what());
    status = EXIT_FAILURE;
  } catch (...) {
    std::println("Unknown exception occurred");
    status = EXIT_FAILURE;
  }

  return status;
}
