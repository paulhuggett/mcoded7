# MIDI Mcoded7 Codecs

An implementation of MIDI Mcoded7 encoder and decoder.

The code consists of a header file containing two clases `mcoded7::encoder` and `mcoded7::decoder`. Their 
interface is identical: for each input byte, a client should call the `parse_byte()` member function 
passing the input byte itself and an `OutputIterator` instance to which the output will be written. Once
all of the input has been passed, call `flush()` again passing an output iterator.

The unit test code uses a [std::back_insert_iterator](https://en.cppreference.com/w/cpp/iterator/back_insert_iterator) 
for convenience, but any object (including a raw pointer) that is compatible with 
[LegacyOutputIterator](https://en.cppreference.com/w/cpp/named_req/OutputIterator) may be used for the 
output iterator argument.

The code includes both unit- and fuzz-tests for strong test coverage.
