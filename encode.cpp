#include <cstdio>
#include <cstring>

#include "mcoded7.hpp"

namespace {

template <typename Codec> bool encode () {
  std::array<std::uint8_t, Codec::max_size> buffer;
  auto *const data = buffer.data ();
  Codec codec;
  int ch;
  while ((ch = std::getchar ()) != EOF) {
    auto *const out = codec.parse_byte (ch, data);
    std::fwrite (data, sizeof (std::uint8_t), out - data, stdout);
  }

  return codec.good ();
}

}  // end anonymous namespace

int main (int argc, char *argv[]) {
  int exit_code = EXIT_SUCCESS;
  try {
    enum class operating_mode { encode, decode };
    auto encode_mode = true;
    if (argc > 1) {
      // Super naive option parser. Use -d for decode and -e for encode.
      if (std::strcmp (argv[1], "-e") == 0) {
        encode_mode = true;
      } else if (std::strcmp (argv[1], "-d") == 0) {
        encode_mode = false;
      } else if (std::strcmp (argv[1], "-h") == 0 || std::strcmp (argv[1], "--help") == 0) {
        std::printf ("Usage: %s {-e|-d}\nOptions:\n\t-e: encode mode\n\t-d: decode mode\n",
                     argv[0]);
        return EXIT_SUCCESS;
      } else {
        std::printf ("Unknown option: %s\n", argv[1]);
        return EXIT_FAILURE;
      }
    }
    bool const good = encode_mode ? encode<mcoded7::encoder> () : encode<mcoded7::decoder> ();
    if (!good) {
      exit_code = EXIT_FAILURE;
    }
  } catch (std::exception const &ex) {
    std::fprintf (stderr, "Error: %s\n", ex.what ());
    exit_code = EXIT_FAILURE;
  } catch (...) {
    std::fprintf (stderr, "Unknown error\n");
    exit_code = EXIT_FAILURE;
  }
  return exit_code;
}
