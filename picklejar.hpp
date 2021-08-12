#ifndef PICKLEJAR_H  // This is the include guard macro
#define PICKLEJAR_H 1
#include <array>
#include <cstring>
#include <fstream>
#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

/*

  Copyright 2021 Pedro Tomas Guillen

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

namespace picklejar {
// START WRITE_API
// START object_stream_v1
template <typename Type>
[[nodiscard]] auto write_object_to_stream(const Type &object,
                                          std::ofstream &ofs_output_file)
    -> bool {
  ofs_output_file.write(reinterpret_cast<const char *>(&object),
                        sizeof(Type));  // NOLINT
  return !ofs_output_file.bad();
}
// END object_stream_v1
// START object_file_v1
template <typename Type>
[[nodiscard]] auto write_object_to_file(const Type &object,
                                        const std::string file_name) -> bool {
  std::ofstream ofs_output_file(
      file_name, std::ios::out | std::ios::trunc | std::ios::binary);
  bool result{write_object_to_stream(object, ofs_output_file)};
  ofs_output_file.close();
  return result;
}
// END object_file_v1
// START object_buffer_v1_array
template <class Type, std::size_t N = sizeof(Type)>
[[nodiscard]] constexpr auto write_object_to_buffer_array(const Type &object)
    -> std::array<char, N> {
  std::array<char, N> buffer_vector_copy_test{};
  std::memcpy(buffer_vector_copy_test.data(), &object, N);
  return buffer_vector_copy_test;
}
// END object_buffer_v1_array
// START object_buffer_v1_vector
template <typename Type>
[[nodiscard]] auto write_object_to_buffer(const Type &object)
    -> std::vector<char> {
  std::vector<char> buffer_vector_copy_test(sizeof(Type));
  std::memcpy(buffer_vector_copy_test.data(), &object, sizeof(Type));
  return buffer_vector_copy_test;
}
// END object_buffer_v1_vector

// START stream_v1
template <typename Type>
auto write_vector_to_stream(const std::vector<Type> &container_of_type,
                            std::ofstream &ofs_output_file) -> bool {
  ofs_output_file.write(
      reinterpret_cast<const char *>(container_of_type.data()),  // NOLINT
      static_cast<long int>(sizeof(Type) * container_of_type.size()));
  return !ofs_output_file.bad();
}
// END stream_v1
// START file_v1
template <typename Type>
auto write_vector_to_file(const std::vector<Type> &container_of_type,
                          const std::string file_name) -> bool {
  std::ofstream ofs_output_file(
      file_name, std::ios::out | std::ios::trunc | std::ios::binary);
  bool result{write_vector_to_stream(container_of_type, ofs_output_file)};
  ofs_output_file.close();
  return result;
}

// END file_v1
// START buffer_v1_array
template <class Type, std::size_t N>
[[nodiscard]] constexpr auto write_vector_to_buffer(
    std::array<Type, N> &vector_output_data)
    -> std::array<char, N * sizeof(Type)> {
  std::array<char, N * sizeof(Type)> buffer_vector_copy_test{};
  std::memcpy(buffer_vector_copy_test.data(), vector_output_data.data(),
              N * sizeof(Type));
  return buffer_vector_copy_test;
}
// END buffer_v1_array
// START buffer_v1_vector
template <class Type>
[[nodiscard]] auto write_vector_to_buffer(std::vector<Type> &vector_output_data)
    -> std::vector<char> {
  const size_t vector_byte_size = vector_output_data.size() * sizeof(Type);
  // std::array<char, vector_byte_size> buffer_vector_copy_test{};
  std::vector<char> buffer_vector_copy_test(vector_byte_size);
  std::memcpy(buffer_vector_copy_test.data(), vector_output_data.data(),
              vector_byte_size);
  return buffer_vector_copy_test;
}
// END buffer_v1_vector
// END WRITE_API

// START READ_API_HELPERS
[[nodiscard]] inline auto ifstream_filesize(std::ifstream &ifstream_input_file)
    -> std::streamsize {
  auto previous_pos = ifstream_input_file.tellg();
  ifstream_input_file.ignore(std::numeric_limits<std::streamsize>::max());
  std::streamsize file_gcount = ifstream_input_file.gcount();
  ifstream_input_file.clear();  //  Since ignore will have set eof.
  ifstream_input_file.seekg(previous_pos, std::ios_base::beg);
  return file_gcount;
}
[[nodiscard]] inline auto ifstream_is_invalid(
    std::ifstream &ifstream_input_file) -> bool {
  return !ifstream_input_file.good();
}
[[nodiscard]] inline auto ifstream_close_and_check_is_invalid(
    std::ifstream &ifstream_input_file) -> bool {
  ifstream_input_file.close();
  return !ifstream_input_file.good();
}

template <class Type>
[[nodiscard]] inline auto
ifstream_is_sizeof_type_larger_than_remaining_readbytes(
    std::ifstream &ifstream_input_file, size_t file_size) -> bool {
  return size_t(ifstream_input_file.tellg()) + sizeof(Type) > file_size or
         ifstream_input_file.eof();
}
// ---------------------- UTILITY MACRO FOR deleting unneeded stuff from class
// NOLINTNEXTLINE
#define DELETE_UNNEEDED_DEFAULTS(class_name)            \
  class_name(class_name &&) = delete;                   \
  class_name(const class_name &) = delete;              \
  auto operator=(class_name &&)->class_name & = delete; \
  auto operator=(const class_name &)->class_name & = delete;
// NOLINTNEXTLINE
#define ADD_MULTIARG_AND_TUPLE_CONSTRUCTORS(class_name, storage)               \
  template <class... Args>                                                     \
  explicit class_name(Args &&...args)                                          \
      : pointer_to_copy{new (&storage) Type{std::forward<Args>(args)...}} {}   \
  template <class Tuple, std::size_t... I>                                     \
  explicit class_name(Tuple &&arg_tuple, std::index_sequence<I...> /*is*/)     \
      : pointer_to_copy{new (&storage) Type{                                   \
            std::get<I>(std::forward<Tuple>(arg_tuple))...}} {}                \
  template <class Tuple, std::size_t... I>                                     \
  explicit class_name(Tuple &&arg_tuple)                                       \
      : class_name(std::forward<Tuple>(arg_tuple),                             \
                   std::make_index_sequence<                                   \
                       std::tuple_size_v<std::remove_reference_t<Tuple>>>{}) { \
  }

// https://en.cppreference.com/w/cpp/language/new
template <class Type>
class ManagedAlignedStorageCopy {
  std::aligned_storage_t<sizeof(Type), alignof(Type)> storage;
  Type *const pointer_to_copy{nullptr};

 public:
  DELETE_UNNEEDED_DEFAULTS(ManagedAlignedStorageCopy)
  ManagedAlignedStorageCopy() : pointer_to_copy{new (&storage) Type} {}
  ADD_MULTIARG_AND_TUPLE_CONSTRUCTORS(ManagedAlignedStorageCopy, storage)
  ~ManagedAlignedStorageCopy() { (*pointer_to_copy).~Type(); }
  auto get_pointer_to_copy() -> Type * { return pointer_to_copy; }
};

template <class Type>
class ManagedAlignedBufferCopy {
  alignas(Type) unsigned char buf[sizeof(Type)]{};
  Type *const pointer_to_copy{nullptr};

 public:
  DELETE_UNNEEDED_DEFAULTS(ManagedAlignedBufferCopy)
  ManagedAlignedBufferCopy() : pointer_to_copy{new (&buf) Type} {}
  ADD_MULTIARG_AND_TUPLE_CONSTRUCTORS(ManagedAlignedBufferCopy, buf)
  ~ManagedAlignedBufferCopy() { (*pointer_to_copy).~Type(); }
  auto get_pointer_to_copy() -> Type * { return pointer_to_copy; }
};

template <class Type>
struct UnionHolder {
  DELETE_UNNEEDED_DEFAULTS(UnionHolder)
  Type value_held;
  UnionHolder(){};   // both dtor and ctor need to be declared empty like this
  ~UnionHolder(){};  // we are basically telling c++ we will manage Type so no
                     // default Type ctor or ~Type dtor are called
};
// DEPRECATED THIS TYPE OF MANAGER CAUSES DOUBLE FREE when running
// the picklejar tests
template <class Type>
class ManagedAlignedUnionCopy {
  UnionHolder<Type> holder{};
  Type *const pointer_to_copy{nullptr};

 public:
  DELETE_UNNEEDED_DEFAULTS(ManagedAlignedUnionCopy)
  ManagedAlignedUnionCopy() : pointer_to_copy{new (&holder.value_held) Type} {}
  ADD_MULTIARG_AND_TUPLE_CONSTRUCTORS(ManagedAlignedUnionCopy,
                                      holder.value_held)

  ~ManagedAlignedUnionCopy() { holder.value_held.~Type(); }
  auto get_pointer_to_copy() -> Type * { return &holder.value_held; }
};
#define ManagedAlignedCopyDefault ManagedAlignedStorageCopy
// END READ_API_HELPERS
// START CONCEPTS
template <typename C, typename T>
concept ContainerHasPushBack = requires(C a, T type) {
  { C() } -> std::same_as<C>;
  { a.push_back(type) } -> std::same_as<void>;
};
template <typename C>
concept ContainerHasDataAndSize = requires(C a) {
  { C() } -> std::same_as<C>;
  { a.size() } -> std::same_as<typename C::size_type>;
  { a.data() } -> std::same_as<typename C::value_type *>;
};
template <typename C>
concept TriviallyCopiable = std::is_trivially_copyable_v<C>;

// NOLINTNEXTLINE
#define CONTAINERWITHHASPUSHBACK_MSG                                        \
  "PICKLEJAR_HELP: You need to pass a container<Type> as first param that " \
  "has .push_back() Ex: std::vector"
// NOLINTNEXTLINE
#define CONTAINERWITHHASDATAANDSIZE_MSG                                      \
  "PICKLEJAR_HELP: You need to pass a container<Type> as second param that " \
  "has .data() and .size() Ex: std::vector or std::array"
// NOLINTNEXTLINE
#define TRIVIALLYCOPIABLE_MSG                                                  \
  "PICKLEJAR_HELP: The object passed to picklejar is not trivially copiable. " \
  "Use of "                                                                    \
  "the other versions of this function may be needed and then a call to "      \
  "'preserve_blank_instance_member' and 'copy_new_bytes_to_instance' in the "  \
  "lambda to fix non trivially copiable members. SEE NON TRIVIAL EXAMPLES "    \
  "section in the readme file"
// END CONCEPTS

// START READ_API
// START object_stream_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
[[nodiscard]] auto operation_specific_read_object_from_stream(
    ManagedAlignedCopy &copy, std::ifstream &ifstream_input_file)
    -> ManagedAlignedCopy & {
  static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  ifstream_input_file.read(
      reinterpret_cast<char *>(copy.get_pointer_to_copy()),  // NOLINT
      sizeof(Type));
  return copy;
}
// END object_stream_v1
// START object_stream_v1_copy uses object_stream_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
[[nodiscard]] auto read_object_from_stream(std::ifstream &ifstream_input_file)
    -> std::optional<Type> {
  static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  ManagedAlignedCopy copy{};
  auto result{std::make_optional<Type>(
      *(operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
            copy, ifstream_input_file))
           .get_pointer_to_copy())};
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  return result;
}
// END object_stream_v1_copy uses object_stream_v1

// START object_file_v1 uses object_stream_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
[[nodiscard]] auto read_object_from_file(ManagedAlignedCopy &copy,
                                         const std::string file_name)
    -> std::optional<ManagedAlignedCopy *> {
  static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{std::make_optional(
      &operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
          copy, ifstream_input_file))};
  if (ifstream_close_and_check_is_invalid(ifstream_input_file)) {
    return {};
  }
  return result;
}
// END object_file_v1 uses object_stream_v1
// START object_file_v1_copy uses object_stream_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
[[nodiscard]] auto read_object_from_file(const std::string file_name)
    -> std::optional<Type> {
  static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  ManagedAlignedCopy copy{};
  auto lower_level_return{
      read_object_from_file<Type, ManagedAlignedCopy>(copy, file_name)};
  if (lower_level_return)
    return std::make_optional(
        *(lower_level_return.value())->get_pointer_to_copy());
  return {};
}
// END object_file_v1_copy uses object_stream_v1

// START stream_v1 uses object_stream_v1
template <typename Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container>
[[nodiscard]] auto read_vector_from_stream(Container &vector_input_data,
                                           std::ifstream &ifstream_input_file)
    -> std::optional<Container> {
  static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  static_assert(ContainerHasPushBack<Container, Type>,
                CONTAINERWITHHASPUSHBACK_MSG);
  if (ifstream_is_invalid(
          ifstream_input_file)) {  // important to check this before
                                   // ifstream_filesize because it clears state
    return {};
  }
  auto file_size = ifstream_filesize(ifstream_input_file);
  auto initial_vector_size = vector_input_data.size();
  // std::puts(("file_length: " + std::to_string(file_size)).c_str());
  // std::puts(("sizeof Type: " + std::to_string(sizeof(Type))).c_str());
  while (ifstream_input_file) {
    if (ifstream_is_invalid(ifstream_input_file)) {
      return {};
    }
    if (ifstream_is_sizeof_type_larger_than_remaining_readbytes<Type>(
            ifstream_input_file, file_size))
      break;
    // std::puts(("gtell" +
    // std::to_string(ifstream_input_file.tellg())).c_str()); // tells us which
    // character we are going to read next
    ManagedAlignedCopy copy{};

    vector_input_data.push_back(std::move(
        *(operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
              copy, ifstream_input_file))
             .get_pointer_to_copy()));
  }
  if (vector_input_data.size() > initial_vector_size) {
    return std::make_optional<Container>(std::move(vector_input_data));
  }
  return {};
}
// END stream_v1 uses object_stream_v1
// START file_v1 uses stream_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container = std::vector<Type>>
[[nodiscard]] auto read_vector_from_file(const std::string file_name)
    -> std::optional<Container> {
  static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  static_assert(ContainerHasPushBack<Container, Type>,
                CONTAINERWITHHASPUSHBACK_MSG);
  Container vector_input_data;
  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{read_vector_from_stream<Type, ManagedAlignedCopy>(
      vector_input_data, ifstream_input_file)};
  if (ifstream_close_and_check_is_invalid(ifstream_input_file)) {
    return {};
  }
  return result;
}
// END file_v1 uses stream_v1

// START object_stream_v2_nochecks
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
auto operation_specific_read_object_from_stream(
    ManagedAlignedCopy &copy, std::ifstream &ifstream_input_file,
    auto &&operation_modify_using_previous_bytes) -> ManagedAlignedCopy & {
  std::array<char, sizeof(Type)> valid_bytes_from_new_blank_instance{};
  std::memcpy(valid_bytes_from_new_blank_instance.data(),
              copy.get_pointer_to_copy(), sizeof(Type));
  std::array<char, sizeof(Type)> bytes_from_file{};
  ifstream_input_file.read(bytes_from_file.data(), sizeof(Type));
  operation_modify_using_previous_bytes(
      *copy.get_pointer_to_copy(), *valid_bytes_from_new_blank_instance.data(),
      *bytes_from_file.data());
  return copy;
}
// END object_stream_v2_nochecks

// START object_stream_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
auto read_object_from_stream(std::ifstream &ifstream_input_file,
                             auto &&operation_modify_using_previous_bytes)
    -> std::optional<Type> {
  ManagedAlignedCopy copy{};
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{
      operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
          copy, ifstream_input_file, operation_modify_using_previous_bytes)
          .get_pointer_to_copy()};
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  return std::make_optional(*result);
}
// END object_stream_v2

// START object_file_v2_nochecks uses object_stream_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
auto operation_specific_read_object_from_file(
    ManagedAlignedCopy &copy, const std::string file_name,
    auto &&operation_modify_using_previous_bytes) -> ManagedAlignedCopy & {
  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  return operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
      copy, ifstream_input_file, operation_modify_using_previous_bytes);
}
// END object_file_v2_nochecks uses object_stream_v2
// START object_file_v2 uses object_stream_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
auto read_object_from_file(ManagedAlignedCopy &copy,
                           const std::string file_name,
                           auto &&operation_modify_using_previous_bytes)
    -> std::optional<ManagedAlignedCopy *> {
  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{
      &operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
          copy, ifstream_input_file, operation_modify_using_previous_bytes)};
  if (ifstream_close_and_check_is_invalid(ifstream_input_file)) {
    return {};
  }
  return std::make_optional(result);
}
// END object_file_v2 uses object_stream_v2
// START object_file_v2_copy uses object_file_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
auto read_object_from_file(const std::string file_name,
                           auto &&operation_modify_using_previous_bytes)
    -> std::optional<Type> {
  ManagedAlignedCopy copy{};
  auto lower_level_return{read_object_from_file<Type, ManagedAlignedCopy>(
      copy, file_name, operation_modify_using_previous_bytes)};
  if (lower_level_return)
    return std::make_optional(
        *(lower_level_return.value())->get_pointer_to_copy());
  return {};
}
// END object_file_v2_copy uses object_file_v2

// START object_buffer_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class BufferContainer>
auto operation_specific_read_object_from_buffer(
    ManagedAlignedCopy &copy, BufferContainer &buffer_with_input_bytes,
    size_t &bytes_read_so_far, auto &&operation_modify_using_previous_bytes)
    -> ManagedAlignedCopy & {
  static_assert(ContainerHasDataAndSize<BufferContainer>,
                CONTAINERWITHHASDATAANDSIZE_MSG);

  std::array<char, sizeof(Type)> valid_bytes_from_new_blank_instance{};
  std::memcpy(valid_bytes_from_new_blank_instance.data(),
              copy.get_pointer_to_copy(), sizeof(Type));
  std::array<char, sizeof(Type)>
      bytes_from_file{};  // TODO(tom): this may be unnecessary we could just
                          // pass buffer_with_input_bytes with offset and size
  std::memcpy(bytes_from_file.data(),
              buffer_with_input_bytes.data() + bytes_read_so_far, sizeof(Type));
  bytes_read_so_far += sizeof(Type);
  operation_modify_using_previous_bytes(
      *copy.get_pointer_to_copy(), *valid_bytes_from_new_blank_instance.data(),
      *bytes_from_file.data());
  return copy;
}
// END object_buffer_v2
// START object_buffer_v2_copy uses object_buffer_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class BufferContainer>
auto read_object_from_buffer(BufferContainer &buffer_with_input_bytes,
                             size_t &bytes_read_so_far,
                             auto &&operation_modify_using_previous_bytes)
    -> Type {
  static_assert(ContainerHasDataAndSize<BufferContainer>,
                CONTAINERWITHHASDATAANDSIZE_MSG);
  ManagedAlignedCopy copy{};
  return *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
               copy, buffer_with_input_bytes, bytes_read_so_far,
               operation_modify_using_previous_bytes))
              .get_pointer_to_copy();
}
// END object_buffer_v2_copy uses object_buffer_v2

// START object_stream_v3 uses object_stream_v2
// this returns a copy, object_stream_v2 be more efficient because it takes
// and returns a reference therefore only generating a move
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
[[nodiscard]] auto read_object_from_stream(
    std::ifstream &ifstream_input_file,
    auto &&operation_modify_using_previous_bytes, auto &&constructor_generator)
    -> std::optional<Type> {
  ManagedAlignedCopy copy{constructor_generator()};
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{std::make_optional(
      *(operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
            copy, ifstream_input_file, operation_modify_using_previous_bytes))
           .get_pointer_to_copy())};
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  return result;
}
// END object_stream_v3 uses object_stream_v2
// START object_file_v3 uses uses object_file_v2
// this returns a copy, object_stream_v2 be more efficient because it takes
// and returns a reference therefore only generating a move
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
[[nodiscard]] auto read_object_from_file(
    const std::string file_name, auto &&operation_modify_using_previous_bytes,
    auto &&constructor_generator) -> std::optional<Type> {
  ManagedAlignedCopy copy{constructor_generator()};
  auto lower_level_return{read_object_from_file<Type, ManagedAlignedCopy>(
      copy, file_name, operation_modify_using_previous_bytes)};
  if (lower_level_return)
    return std::make_optional(
        *(lower_level_return.value())->get_pointer_to_copy());
  return {};
}
// END object_file_v3 uses object_file_v2
// START object_buffer_v3 uses object_buffer_v2
// this returns a copy, object_buffer_v2 be more efficient because it takes
// and returns a reference therefore only generating a move
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class BufferContainer>
[[nodiscard]] auto read_object_from_buffer(
    BufferContainer &buffer_with_input_bytes, size_t &bytes_read_so_far,
    auto &&operation_modify_using_previous_bytes, auto &&constructor_generator)
    -> Type {
  static_assert(ContainerHasDataAndSize<BufferContainer>,
                CONTAINERWITHHASDATAANDSIZE_MSG);
  ManagedAlignedCopy copy{constructor_generator()};
  return *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
               copy, buffer_with_input_bytes, bytes_read_so_far,
               operation_modify_using_previous_bytes))
              .get_pointer_to_copy();
}
// END object_buffer_v3 uses object_buffer_v2
namespace util {
void preserve_blank_instance_member(size_t blank_instance_member_offset,
                                    size_t blank_instance_member_size,
                                    auto &valid_bytes_from_new_blank_instance,
                                    auto &bytes_from_file) {
  // copy valid std::string bytes to the bytes we read from the file
  std::memcpy(
      &bytes_from_file + blank_instance_member_offset,
      &valid_bytes_from_new_blank_instance + blank_instance_member_offset,
      blank_instance_member_size);
}
void copy_new_bytes_to_instance(auto &bytes_from_file, auto &blank_instance,
                                size_t size_of_object) {
  // copy our read bytes to our blank copy
  std::memcpy(&blank_instance, &bytes_from_file, size_of_object);
}
}  // namespace util

// START object_buffer_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class BufferContainer>
[[nodiscard]] auto operation_specific_read_object_from_buffer(
    ManagedAlignedCopy &copy, BufferContainer &buffer_with_input_bytes,
    size_t &bytes_read_so_far) -> ManagedAlignedCopy & {
  static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  static_assert(ContainerHasDataAndSize<BufferContainer>,
                CONTAINERWITHHASDATAANDSIZE_MSG);
  util::copy_new_bytes_to_instance(
      *(buffer_with_input_bytes.data() + bytes_read_so_far),
      *copy.get_pointer_to_copy(), sizeof(Type));
  bytes_read_so_far += sizeof(Type);
  return copy;
}
// END object_buffer_v1
// START object_buffer_v1_copy uses object_buffer_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class BufferContainer>
[[nodiscard]] auto read_object_from_buffer(
    BufferContainer &buffer_with_input_bytes, size_t &bytes_read_so_far)
    -> Type {
  static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  static_assert(ContainerHasDataAndSize<BufferContainer>,
                CONTAINERWITHHASDATAANDSIZE_MSG);
  ManagedAlignedCopy copy{};
  return *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
               copy, buffer_with_input_bytes, bytes_read_so_far))
              .get_pointer_to_copy();
}
// END object_buffer_v1_copy uses object_buffer_v1

// START buffer_v1 uses object_buffer_v1
// BUFFER VERSION taken from OPERATION VERSION
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class BufferContainer, class Container>
[[nodiscard]] constexpr auto read_vector_from_buffer(
    Container &vector_input_data, BufferContainer &buffer_with_input_bytes)
    -> std::optional<Container> {
  static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  static_assert(ContainerHasPushBack<Container, Type>,
                CONTAINERWITHHASPUSHBACK_MSG);
  static_assert(ContainerHasDataAndSize<BufferContainer>,
                CONTAINERWITHHASDATAANDSIZE_MSG);
  auto file_size = buffer_with_input_bytes.size();
  auto initial_vector_size = vector_input_data.size();
  // std::puts(("file_length: " + std::to_string(file_size)).c_str());
  // std::puts(("sizeof Type: " + std::to_string(sizeof(Type))).c_str());
  if (file_size < 1) {
    return {};
  }
  size_t bytes_read_so_far = 0;
  while (bytes_read_so_far < file_size) {
    if (bytes_read_so_far + sizeof(Type) >
        static_cast<unsigned long>(file_size))
      break;
    // START OPERATION VERSION
    ManagedAlignedCopy copy{};
    // END OPERATION VERSION
    vector_input_data.push_back(std::move(
        *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
              copy, buffer_with_input_bytes, bytes_read_so_far))
             .get_pointer_to_copy()));
  }
  if (vector_input_data.size() > initial_vector_size) {
    return std::make_optional<Container>(std::move(vector_input_data));
  }
  return {};
}
// END buffer_v1 uses object_buffer_v1
// START buffer_v3 uses object_buffer_v2
// BUFFER VERSION taken from OPERATION VERSION
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class BufferContainer, class Container>
[[nodiscard]] constexpr auto read_vector_from_buffer(
    Container &vector_input_data, BufferContainer &buffer_with_input_bytes,
    auto &&operation_modify_using_previous_bytes, auto &&constructor_generator)
    -> std::optional<Container> {
  static_assert(ContainerHasDataAndSize<BufferContainer>,
                CONTAINERWITHHASDATAANDSIZE_MSG);
  auto file_size = buffer_with_input_bytes.size();
  auto initial_vector_size = vector_input_data.size();
  // std::puts(("file_length: " + std::to_string(file_size)).c_str());
  // std::puts(("sizeof Type: " + std::to_string(sizeof(Type))).c_str());
  if (file_size < 1) {
    return {};
  }
  size_t bytes_read_so_far = 0;
  while (bytes_read_so_far < file_size) {
    std::puts(
        ("bytes_read_so_far: " + std::to_string(bytes_read_so_far)).c_str());
    if (bytes_read_so_far + sizeof(Type) >
        static_cast<unsigned long>(file_size))
      break;
    // START OPERATION VERSION
    ManagedAlignedCopy copy{constructor_generator()};

    // END OPERATION VERSION
    vector_input_data.push_back(std::move(
        *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
              copy, buffer_with_input_bytes, bytes_read_so_far,
              operation_modify_using_previous_bytes))
             .get_pointer_to_copy()));
  }
  if (vector_input_data.size() > initial_vector_size) {
    return std::make_optional<Container>(std::move(vector_input_data));
  }
  return {};
}
// END buffer_v3 uses object_buffer_v2
// START stream_v3 uses object_stream_v2
// OPERATION VERSION
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container>
[[nodiscard]] auto read_vector_from_stream(
    Container &vector_input_data, std::ifstream &ifstream_input_file,
    auto &&operation_modify_using_previous_bytes, auto &&constructor_generator)
    -> std::optional<Container> {
  if (ifstream_is_invalid(
          ifstream_input_file)) {  // important to check this before
                                   // ifstream_filesize because it clears state
    return {};
  }
  auto file_size = ifstream_filesize(ifstream_input_file);
  auto initial_vector_size = vector_input_data.size();
  if (file_size < 1) {
    return {};
  }
  // std::puts(("file_length: " + std::to_string(file_size)).c_str());
  // std::puts(("sizeof Type: " + std::to_string(sizeof(Type))).c_str());
  // vector_input_data.reserve(5);
  while (ifstream_input_file) {
    if (ifstream_is_invalid(ifstream_input_file)) {
      return {};
    }
    if (ifstream_is_sizeof_type_larger_than_remaining_readbytes<Type>(
            ifstream_input_file, file_size))
      break;
    ManagedAlignedCopy copy{constructor_generator()};
    vector_input_data.push_back(std::move(
        *(operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
              copy, ifstream_input_file, operation_modify_using_previous_bytes))
             .get_pointer_to_copy()));
  }
  if (vector_input_data.size() > initial_vector_size) {
    return std::make_optional<Container>(std::move(vector_input_data));
  }
  return {};
}
// END stream_v3 uses object_stream_v2
// START stream_v2 uses stream_v3
constexpr auto return_empty_tuple = []() { return std::tuple(); };

template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container>
[[nodiscard]] auto read_vector_from_stream(
    Container &vector_input_data, std::ifstream &ifstream_input_file,
    auto &&operation_modify_using_previous_bytes) -> std::optional<Container> {
  return read_vector_from_stream<Type, ManagedAlignedCopy>(
      vector_input_data, ifstream_input_file,
      operation_modify_using_previous_bytes, return_empty_tuple);
}
// END stream_v2 uses stream_v3
// START buffer_v2 uses buffer_v3
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container, class BufferContainer>
[[nodiscard]] constexpr auto read_vector_from_buffer(
    Container &vector_input_data, BufferContainer &buffer_with_input_bytes,
    auto &&operation_modify_using_previous_bytes) -> std::optional<Container> {
  static_assert(ContainerHasDataAndSize<BufferContainer>,
                CONTAINERWITHHASDATAANDSIZE_MSG);
  return read_vector_from_buffer<Type, ManagedAlignedCopy>(
      vector_input_data, buffer_with_input_bytes,
      operation_modify_using_previous_bytes, return_empty_tuple);
}
// END buffer_v2 uses buffer_v3
// START file_v2 uses stream_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container = std::vector<Type>>
[[nodiscard]] auto read_vector_from_file(
    const std::string file_name, auto &&operation_modify_using_previous_bytes)
    -> std::optional<Container> {
  Container vector_input_data;

  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{read_vector_from_stream<Type, ManagedAlignedCopy>(
      vector_input_data, ifstream_input_file,
      operation_modify_using_previous_bytes)};
  if (ifstream_close_and_check_is_invalid(ifstream_input_file)) {
    return {};
  }
  return result;
}
// END file_v2 uses stream_v2
// START file_v3 uses stream_v3
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container = std::vector<Type>>
[[nodiscard]] auto read_vector_from_file(
    const std::string file_name, auto &&operation_modify_using_previous_bytes,
    auto &&constructor_generator) -> std::optional<Container> {
  Container vector_input_data;
  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{read_vector_from_stream<Type, ManagedAlignedCopy>(
      vector_input_data, ifstream_input_file,
      operation_modify_using_previous_bytes, constructor_generator)};
  if (ifstream_close_and_check_is_invalid(ifstream_input_file)) {
    return {};
  }
  return result;
}
// END file_v3 uses stream_v3
// END READ_API
}  // namespace picklejar
#endif
