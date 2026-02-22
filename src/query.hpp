#pragma once

namespace mewo::query {

consteval bool is_debug()
{
#if defined(MEWO_IS_DEBUG)
  return true;
#else
  return false;
#endif
}

consteval bool is_release() { return !is_debug(); }

}
