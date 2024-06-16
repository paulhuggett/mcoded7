#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>

// MIDI Mcoded7 Encoding
// ~~~~~~~~~~~~~~~~~~~~~
// Each group of seven stored bytes is transmitted as eight bytes. First, the sign bits of the seven
// bytes are sent, followed by the low-order 7 bits of each byte. (The reasoning is that this would
// make the auxiliary bytes appear in every 8th byte without exception, which would therefore be
// slightly easier for the receiver to decode.)
//
// The seven bytes:
//     AAAAaaaa BBBBbbbb CCCCcccc DDDDdddd EEEEeeee FFFFffff GGGGgggg
// are sent as:
//     0ABCDEFG
//     0AAAaaaa 0BBBbbbb 0CCCcccc 0DDDdddd 0EEEeeee 0FFFffff 0GGGgggg
//
// From a buffer to be encoded, complete groups of seven bytes are encoded into groups of eight
// bytes. If the buffer size is not a multiple of seven, there will be some number of bytes leftover
// after the groups of seven are encoded. This short group is transmitted similarly, with the sign
// bits occupying the most significant bits of the first transmitted byte. For example:
//     AAAAaaaa BBBBbbbb CCCCcccc
// are transmitted as:
//     0ABC0000 0AAAaaaa 0BBBbbbb 0CCCcccc

namespace mcoded7 {

class encoder {
public:
  /// \tparam OutputIterator  An output iterator type to which bytes can be written.
  /// \param value  The value to be encoded.
  /// \param out  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <typename OutputIterator>
  OutputIterator parse_byte (std::uint8_t const value, OutputIterator out) {
    assert (pos_ < 7U && "on entry, pos_ must be in the range [0,7)");
    static constexpr auto msb = 0x80;
    ++pos_;
    buffer_[0] |= (value & msb) >> pos_;
    buffer_[pos_] = value & ~msb;
    if (pos_ == 7U) {
      out = this->flush (out);
    }
    return out;
  }
  /// Call once the entire input sequence has been fed to encoder::parse_byte(). This function
  /// flushes any remaining buffered output.
  ///
  /// \tparam OutputIterator  An output iterator type to which bytes can be written.
  /// \param out  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <typename OutputIterator> OutputIterator flush (OutputIterator out) {
    if (pos_ > 0U) {
      auto const first = std::begin (buffer_);
      out = std::copy (first, first + pos_ + 1, out);
      buffer_[0] = 0U;  // reset the MSB for the next block of 7.
      pos_ = 0U;
    }
    return out;
  }

private:
  std::array<std::uint8_t, 8> buffer_{};
  std::uint8_t pos_ = 0;
};

class decoder {
public:
  /// \tparam OutputIterator  An output iterator type to which bytes can be written.
  /// \param value  The value to be decoded.
  /// \param out  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <typename OutputIterator>
  OutputIterator parse_byte (std::uint8_t const value, OutputIterator out) {
    assert (pos_ < 8U && "On entry, pos_ must be in the range [0,8)");
    if (pos_ == 7U) {
      // This the the byte that encodes the sign bits of the seven following bytes.
      msbs_ = value;
    } else {
      assert (static_cast<unsigned> (value & 0x80) == 0 &&
              "Mcoded7 data should always have the most significant bit clear");
      // Assemble the output byte from ths input value and its most-significant sign bit stored in
      // msbs_.
      *(out++) = value | (((msbs_ >> pos_) & 0x01) << 7);
      if (pos_ == 0U) {
        pos_ = 8U;
      }
    }
    --pos_;
    return out;
  }
  /// Call once the entire input sequence has been fed to decoder::parse_byte(). This function
  /// flushes any remaining buffered output.
  ///
  /// \tparam OutputIterator  An output iterator type to which bytes can be written.
  /// \param out  An output iterator to which the output sequence is written.
  /// \returns  Iterator one past the last element assigned.
  template <typename OutputIterator> OutputIterator flush (OutputIterator out) { return out; }

private:
  std::uint8_t msbs_ = 0U;
  std::uint8_t pos_ = 7U;
};

}  // end namespace mcoded7
