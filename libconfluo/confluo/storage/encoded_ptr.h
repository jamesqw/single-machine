#ifndef CONFLUO_STORAGE_ENCODED_PTR_H_
#define CONFLUO_STORAGE_ENCODED_PTR_H_

#include "compression/delta_decoder.h"
#include "compression/lz4_decoder.h"
#include "ptr_metadata.h"

namespace confluo {
namespace storage {

namespace detail {

template<typename T>
static void no_op_delete(T *) {}

template<typename U>
static void array_delete(U *ptr) { std::default_delete<U[]>()(ptr); }

}

// TODO may need to inherit from unique_ptr so we can provide a default
//  constructor in order to make things cleaner for the caller.
template<typename T>
using decoded_ptr = typename std::unique_ptr<T, void (*)(T *)>;

template<typename T>
class encoded_ptr {
 public:
  encoded_ptr(void *ptr = nullptr)
      : ptr_(ptr) {
  }

  /**
   * @return encoded pointer
   */
  void *ptr() const {
    return ptr_;
  }

  /**
   * @return encoded pointer
   */
  template<typename U>
  U *ptr_as() const {
    return static_cast<U *>(ptr_);
  }

  //
  // Encode/decode member functions
  //

  // TODO maybe change encode to write since encoded writes aren't supported?

  /**
   * Encode value and store at index.
   * @param idx index
   * @param val value
   */
  void encode(size_t idx, T val) {
    ptr_aux_block aux = ptr_aux_block::get(ptr_metadata::get(ptr_));
    switch (aux.encoding_) {
      case encoding_type::D_UNENCODED: {
        this->ptr_as<T>()[idx] = val;
        break;
      }
      default: {
        THROW(unsupported_exception, "Writing to an encoded pointer is unsupported!");
      }
    }
  }

  /**
   * Encode data and store in pointer.
   * @param idx index into decoded representation to store at
   * @param data buffer of decoded data to encode and store
   * @param len number of elements of T
   */
  void encode(size_t idx, const T *data, size_t len) {
    ptr_aux_block aux = ptr_aux_block::get(ptr_metadata::get(ptr_));
    switch (aux.encoding_) {
      case encoding_type::D_UNENCODED: {
        memcpy(&this->ptr_as<T>()[idx], data, sizeof(T) * len);
        break;
      }
      default: {
        THROW(illegal_state_exception, "Writing to an encoded pointer is unsupported!");
      }
    }
  }

  /**
   * Decode element at index.
   * @param idx index of data to decode
   * @return deocoded element
   */
  T decode_at(size_t idx) const {
    auto *metadata = ptr_metadata::get(ptr_);
    auto aux = ptr_aux_block::get(metadata);
    switch (aux.encoding_) {
      case encoding_type::D_UNENCODED: {
        return this->ptr_as<T>()[idx];
      }
      case encoding_type::D_ELIAS_GAMMA: {
        T decoded = compression::delta_decoder::decode<T>(this->ptr_as<uint8_t>(), idx);
        return decoded;
      }
      case encoding_type::D_LZ4: {
        T decoded = compression::lz4_decoder<>::decode(this->ptr_as<uint8_t>(), idx);
        return decoded;
      }
      default: {
        THROW(illegal_state_exception, "Invalid encoding type!");
      }
    }
  }

  /**
   * Decode pointer into buffer.
   * @param buffer buffer to store decoded data in
   * @param start_idx index to start at
   * @param len number of elements
   */
  void decode(T *buffer, size_t start_idx, size_t len) const {
    auto aux = ptr_aux_block::get(ptr_metadata::get(ptr_));
    switch (aux.encoding_) {
      case encoding_type::D_UNENCODED: {
        memcpy(buffer, &this->ptr_as<T>()[start_idx], sizeof(T) * len);
        break;
      }
      case encoding_type::D_ELIAS_GAMMA: {
        compression::delta_decoder::decode<T>(this->ptr_as<uint8_t>(), buffer, start_idx, len);
        break;
      }
      case encoding_type::D_LZ4: {
        compression::lz4_decoder<>::decode(this->ptr_as<uint8_t>(),
                                           reinterpret_cast<uint8_t *>(buffer), start_idx, len);
        break;
      }
      default: {
        THROW(illegal_state_exception, "Invalid encoding type!");
      }
    }
  }

  /**
   * Decode pointer and store in a newly allocated buffer,
   * managed by a unique_ptr instance.
   * @param start_idx index to start at
   * @param len number of elements
   * @return pointer to decoded buffer
   */
  std::unique_ptr<T> decode(size_t start_idx, size_t len) const {
    T *decoded = new T[len];
    decode(decoded, start_idx, len);
    return std::unique_ptr<T>(decoded);
  }

  /**
   * Decode pointer, starting at an index.
   * @param start_idx index to start decoding at
   * @return decoded pointer
   */
  decoded_ptr<T> decode(size_t start_idx) const {
    auto *metadata = ptr_metadata::get(ptr_);
    auto aux = ptr_aux_block::get(metadata);
    switch (aux.encoding_) {
      case encoding_type::D_UNENCODED: {
        return decoded_ptr<T>(this->ptr_as<T>() + start_idx, detail::no_op_delete<T>);
      }
      case encoding_type::D_ELIAS_GAMMA: {
        size_t encoded_size = metadata->data_size_;
        size_t decoded_size = compression::delta_decoder::decoded_size(this->ptr_as<uint8_t>());
        T *decoded = new T[decoded_size];
        compression::delta_decoder::decode<T>(this->ptr_as<uint8_t>(), decoded, start_idx);
        return decoded_ptr<T>(decoded, detail::array_delete<T>);
      }
      case encoding_type::D_LZ4: {
        size_t decoded_size = compression::lz4_decoder<>::decoded_size(this->ptr_as<uint8_t>());
        uint8_t *decoded = new uint8_t[decoded_size];
        compression::lz4_decoder<>::decode(this->ptr_as<uint8_t>(), decoded, start_idx);
        return decoded_ptr<T>(reinterpret_cast<T *>(decoded), detail::array_delete<T>);
      }
      default: {
        THROW(illegal_state_exception, "Invalid encoding type!");
      }
    }
  }

 private:
  void *ptr_; // encoded data stored at this pointer

};

}
}

#endif /* CONFLUO_STORAGE_ENCODED_PTR_H_ */
