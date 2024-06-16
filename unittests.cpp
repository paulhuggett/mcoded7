#include "mcoded7.hpp"

// standard library
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MCODED7_FUZZTEST) && MCODED7_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace {

struct raw_and_encoded {
  std::vector<std::byte> raw;
  std::vector<std::byte> encoded;
};

class Mcoded7 : public testing::TestWithParam<raw_and_encoded> {
public:
  /// Takes a vector of bytes and returns the mcoded7 encoded equivalent vector.
  static std::vector<std::byte> encode (std::vector<std::byte> const& input) {
    mcoded7::encoder encoder;
    std::vector<std::byte> output;
    auto out = std::back_inserter (output);
    std::ranges::for_each (
        input, [&encoder, &out] (std::byte const b) { out = encoder.parse_byte (b, out); });
    encoder.flush (out);
    return output;
  }

  /// Decodes a vector of mcoded7 bytes.
  static std::vector<std::byte> decode (std::vector<std::byte> const& input) {
    mcoded7::decoder decoder;
    std::vector<std::byte> output;
    auto out = std::back_inserter (output);
    std::ranges::for_each (
        input, [&decoder, &out] (std::byte const b) { out = decoder.parse_byte (b, out); });
    decoder.flush (out);
    return output;
  }
};

}  // end anonymous namespace

// NOLINTNEXTLINE
TEST_P (Mcoded7, Encode) {
  auto const& param = GetParam ();
  EXPECT_THAT (encode (param.raw), testing::ContainerEq (param.encoded));
}
// NOLINTNEXTLINE
TEST_P (Mcoded7, Decode) {
  auto const& param = GetParam ();
  EXPECT_THAT (decode (param.encoded), testing::ContainerEq (param.raw));
}

namespace {

raw_and_encoded const empty;
raw_and_encoded const four{std::vector<std::byte>{
                               std::byte{0b0001'0010},
                               std::byte{0b0011'0100},
                               std::byte{0b0101'0110},
                               std::byte{0b0111'1000},
                           },
                           std::vector<std::byte>{
                               std::byte{0b0000'0000},  // MSBs
                               std::byte{0b0001'0010},
                               std::byte{0b0011'0100},
                               std::byte{0b0101'0110},
                               std::byte{0b0111'1000},
                           }};
raw_and_encoded const seven{std::vector<std::byte>{
                                std::byte{0b0001'0010},
                                std::byte{0b0011'0100},
                                std::byte{0b0101'0110},
                                std::byte{0b0111'1000},
                                std::byte{0b1001'1010},
                                std::byte{0b1011'1100},
                                std::byte{0b1101'1110},
                            },
                            std::vector<std::byte>{
                                std::byte{0b0000'0111},  // MSBs
                                std::byte{0b0001'0010},
                                std::byte{0b0011'0100},
                                std::byte{0b0101'0110},
                                std::byte{0b0111'1000},
                                std::byte{0b0001'1010},
                                std::byte{0b0011'1100},
                                std::byte{0b0101'1110},
                            }};
raw_and_encoded const eight{std::vector<std::byte>{
                                std::byte{0b0001'0010},
                                std::byte{0b0011'0100},
                                std::byte{0b0101'0110},
                                std::byte{0b0111'1000},
                                std::byte{0b1001'1010},
                                std::byte{0b1011'1100},
                                std::byte{0b1101'1110},
                                std::byte{0b1111'0000},
                            },
                            std::vector<std::byte>{
                                // block #1
                                std::byte{0b0000'0111},  // MSBs for block #1
                                std::byte{0b0001'0010},
                                std::byte{0b0011'0100},
                                std::byte{0b0101'0110},
                                std::byte{0b0111'1000},
                                std::byte{0b0001'1010},
                                std::byte{0b0011'1100},
                                std::byte{0b0101'1110},

                                // block #2
                                std::byte{0b0100'0000},  // MSBs for block #2
                                std::byte{0b0111'0000},
                            }};

}  // end anonymous namespace

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P (Mcoded7, Mcoded7, testing::Values (empty, four, seven, eight));

namespace {

void Mcoded7RoundTrip (std::vector<std::byte> const& input) {
  auto const encoded = Mcoded7::encode (input);
  auto const decoded = Mcoded7::decode (encoded);
  EXPECT_THAT (decoded, testing::ContainerEq (input));
}

}  // end anonymous namespace

#if defined(MCODED7_FUZZTEST) && MCODED7_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST (Mcoded7, Mcoded7RoundTrip);
#endif
// NOLINTNEXTLINE
TEST (Mcoded7, EmptyRoundTrip) {
  Mcoded7RoundTrip (std::vector<std::byte>{});
}
