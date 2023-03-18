#ifndef CONFLUO_COMPRESSION_DELTA_DECODER_H_
#define CONFLUO_COMPRESSION_DELTA_DECODER_H_

#include <cstdint>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <math.h>

#include "container/bitmap/delta_encoded_array.h"

namespace confluo {
namespace compression {

/**
 * A stateless decoder. Takes as input a delta encoded input buffer and
 * performs partial or full decoding.
 */
class delta_decoder {
 public:
  /**
   * Decodes a single element at a given index
   *
   * @param input_buffer The encoded buffer to decode
   * @param src_index The index to decode at
   *
   * @return The decoded element
   */
  template<typename T>
  static T decode(uint8_t *input_buffer, size_t src_index) {
    elias_gamma_encoded_array<T> enc_array;
    enc_array.from_byte_array(input_buffer + sizeof(size_t));
    return enc_array.get(src_index);
  }

  /**
   * Decodes a partial amount of the buffer starting from a specific index
   *
   * @param input_buffer The encoded buffer
   * @param dest_buffer The decoded buffer to contain the decoded bytes
   * @param src_index The index to start decoding from
   * @param length The number of elements to decode
   */
  template<typename T>
  static void decode(uint8_t *input_buffer, T *dest_buffer, size_t src_index, size_t length) {
    elias_gamma_encoded_array<T> enc_array;
    enc_array.from_byte_array(input_buffer + sizeof(size_t));
    for (size_t i = 0; i < length; i++) {
      dest_buffer[i] = enc_array.get(src_index + i);
    }
  }

  /**
   * Decodes the whole pointer starting from the specified index
   *
   * @param input_buffer The encoded buffer
   * @param dest_buffer The buffer containing the decoded bytes
   * @param src_index The index to start decoding from
   */
  template<typename T>
  static void decode(uint8_t *input_buffer, T *dest_buffer, size_t src_index = 0) {
    size_t source_size = decoded_size(input_buffer);
    decode<T>(input_buffer, dest_buffer, src_index, source_size - src_index);
  }

  static size_t decoded_size(uint8_t *input_buffer) {
    return *reinterpret_cast<size_t *>(input_buffer);
  }

};

}
}

#endif /* CONFLUO_COMPRESSION_DELTA_DECODER_H_ */
