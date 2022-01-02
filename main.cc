#include "elf/mold.h"
#include "macho/mold.h"

#include <cstring>
#include <signal.h>

namespace mold {

std::string_view errno_string() {
  // There's a thread-safe version of strerror() (strerror_r()), but
  // GNU and POSIX define that function differently. To avoid the mess,
  // we simply use strerror() with a lock.
  static std::mutex mu;
  std::lock_guard lock(mu);
  return strerror(errno);
}

#ifdef GIT_HASH
const std::string mold_version =
  "mold " MOLD_VERSION " (" GIT_HASH "; compatible with GNU ld and GNU gold)";
#else
const std::string mold_version =
  "mold " MOLD_VERSION " (compatible with GNU ld and GNU gold)";
#endif

void cleanup() {
  if (output_tmpfile)
    unlink(output_tmpfile);
  if (socket_tmpfile)
    unlink(socket_tmpfile);
}

static void signal_handler(int) {
  cleanup();
  _exit(1);
}

void install_signal_handler() {
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
}

} // namespace mold

int main(int argc, char **argv) {
  std::string cmd = mold::filepath(argv[0]).filename();

  if (cmd == "ld64" || cmd == "ld64.mold")
    return mold::macho::main(argc, argv);

  return mold::elf::main(argc, argv);
}
