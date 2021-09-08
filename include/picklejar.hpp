#ifndef PICKLEJAR_H  // This is the include guard macro
#define PICKLEJAR_H 1
#include <array>
#include <cassert>
#include <cstring>
#include <fstream>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// if you want to use this header file only and not have to include the
// type_safe thirdparty library you can define DISABLE_TYPESAFE_OPTIONAL from
// the command line or your header file or cmake.
// The implications are that for functions that take an "in" value
// for example:
// auto read_vector_from_stream(Container &vector_input_data,
//                                           std::ifstream &ifstream_input_file)
// takes vector_input_data as an "in" variable which means vector_input_data is
// modified inside the function.
//
// I wanted to have a way to express this instead of having an "in" variable, so
// we return a std::optional<Container>. The problem with this is the function
// now returns a copy of vector_input_data, so instead we std::move the
// Container into the optional. This is okay, but the ideal case would be to
// return a reference of Container, but std::optional<Container&> cannot be done
// because a reference is not a type. Instead we could do
// std::optional<Container*> and return a pointer but I had existing code that
// used this library and I didn't want to break it so instead we have a version
// with move that uses std::optional<Container> and a version that uses a
// thirdparty type_safe library that allows us to have an optional of a
// reference in functionality. type_safe::optional_ref uses a pointer and acts
// as a reference, therefore not breaking code that was previously using the
// std::optional<Container> version. Both should be interchangeable with the
// exception the type_safe doesn't move the Container variable, instead it would
// modify it in-place and return a reference if successful. As long as you use
// the optional.value() to access the Container after calling the vector
// interfaces your code shouldn't need any changes.
//#define DISABLE_TYPESAFE_OPTIONAL
#ifndef DISABLE_TYPESAFE_OPTIONAL
#include <type_safe/optional_ref.hpp>
#endif

// START PICKLEJAR_ASSERT // taken from
// https://stackoverflow.com/questions/3767869/adding-message-to-assert
#ifndef NDEBUG
#include <iostream>
#define PICKLEJAR_ASSERT(condition, message)                           \
  do {                                                                 \
    if (!(condition)) {                                                \
      std::cerr << "PICKLEJAR_ASSERTION: Condition `" #condition       \
                   "` failed in "                                      \
                << __FILE__ << " line " << __LINE__ << ": " << message \
                << '\n';                                               \
      std::exit(EXIT_FAILURE);                                         \
    }                                                                  \
  } while (false)
#else
#define PICKLEJAR_ASSERT(condition, message) \
  do {                                       \
  } while (false)
#endif
#ifndef NDEBUG
#include <iostream>
#define PICKLEJAR_MESSAGE(condition, message)                               \
  do {                                                                      \
    if (!(condition)) {                                                     \
      std::cerr                                                             \
          << "PICKLEJAR_VERBOSE_MODE: Non-critical condition: `" #condition \
             "` failed in "                                                 \
          << __FILE__ << " line " << __LINE__ << ": " << message << '\n';   \
    }                                                                       \
  } while (false)
#else
#define PICKLEJAR_MESSAGE(condition, message) \
  do {                                        \
  } while (false)
#endif
// END PICKLEJAR_ASSERT

// START FUNCTION TO PRINT THE NAME OF A C++ TYPE TYPE_NAME()
// taken from: https://stackoverflow.com/a/56766138
// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/56766138#56766138
#include <string_view>

template <typename T>
constexpr auto type_name() noexcept {
  std::string_view name = "Error: unsupported compiler", prefix, suffix;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  prefix = "auto type_name() [T = ";
  suffix = "]";
#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  prefix = "constexpr auto type_name() [with T = ";
  suffix = "]";
#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  prefix = "auto __cdecl type_name<";
  suffix = ">(void) noexcept";
#endif
  name.remove_prefix(prefix.size());
  name.remove_suffix(suffix.size());
  return name;
}
// END FUNCTION TO PRINT THE NAME OF A C++ TYPE TYPE_NAME()
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

#ifdef DISABLE_TYPESAFE_OPTIONAL
// Version 1 of PICKLEJAR_MAKE_OPTIONAL uses only std
// The problem with this version of make_optional is we are moving a vector into
// the optional, we could return an optional of a pointer to the vector but it
// would break existing code and I like not having a pointer return value inside
// the optional, which is why we use the type_safe library. The bad or good
// thing in some cases is that when we return something using this version
// std::moves the value out of the "In" variable the function was called with,
// and so you should always use the .value() from the option if successful with
// any of the two versions. If the function is unsuccessful it wont move
// anything and you can still use the other "In" variable from it's own scope
template <class object_type>
using optional = std::optional<object_type>;
#define PICKLEJAR_MAKE_OPTIONAL(object) std::make_optional(std::move(object))
// RETURN_RESULT_FROM_FILE returns a copy in this case
#define RETURN_RESULT_FROM_FILE result.value()
#else
// Version 2 of PICKLEJAR_MAKE_OPTIONAL uses thirdparty type_safe library
// Instead of having "In" variables we return an optional_ref, this should be
// pretty efficient and the returned interface is identical, it doesn't use
// std::move so the "In" variable is still valid in it's scope in contrast with
// the other make_optional version which uses std::move
template <class object_type>
using optional = type_safe::optional_ref<object_type>;
#define PICKLEJAR_MAKE_OPTIONAL(object) type_safe::ref(object)
// RETURN_RESULT_FROM_FILE may use RVO?
#define RETURN_RESULT_FROM_FILE vector_input_data
#endif

// RUNTIME_MESSAGES
#ifndef PICKLEJAR_ENABLE_VERBOSE_MODE
// only non-critical messages will be disabled if you set this to 0
#define PICKLEJAR_ENABLE_VERBOSE_MODE 1
#endif
#define PICKLEJAR_RUNTIME_READSIZE_MISSMATCH                                 \
  "PICKLEJAR_RUNTIME_MESSAGE: The size that was read ("                      \
      << byte_buffer.byte_counter.value()                                    \
      << ") in the 'vector_insert_element_lambda' does NOT match the size "  \
         "that was written to the file ("                                    \
      << optional_size.value()                                               \
      << "). Compare your write and read functions and make sure you are "   \
         "reading all elements correctly including their versions. You may " \
         "disable this check by adding "                                     \
         "\"byte_vector_with_counter.set_counter(byte_vector_with_counter."  \
         "size());\" near the end of the 'vector_insert_element_lambda'"

#define PICKLEJAR_RUNTIME_READ_VERSION_MISSMATCH                         \
  "PICKLEJAR_RUNTIME_MESSAGE: The version from the file ("               \
      << optional_version.value()                                        \
      << ") doesn't match with the Version of the function (" << Version \
      << ")"

#define PICKLEJAR_RUNTIME_BYTEVECTORWITHCOUNTER_BYTE_COUNTER_INVALIDATED       \
  "The byte_counter for this ByteVectorWithCounter has been invalidated, "     \
  "this happened because some part of your code tried to advance the counter " \
  "by ("                                                                       \
      << size_to_advance << ") which is more than it's remaining size ("       \
      << size_remaining() << ")"

// START MANAGEDSTORAGE CLASSES
// ---------------------- UTILITY MACRO FOR deleting unneeded stuff from class
// NOLINTNEXTLINE
#define DELETE_UNNEEDED_DEFAULTS(class_name)            \
  class_name(class_name &&) = delete;                   \
  class_name(const class_name &) = delete;              \
  auto operator=(class_name &&)->class_name & = delete; \
  auto operator=(const class_name &)->class_name & = delete;
// ---------------------- UTILITY MACRO FOR adding constructors to managedstorage classes
// NOLINTNEXTLINE
#define ADD_MULTIARG_AND_TUPLE_CONSTRUCTORS(class_name, storage)               \
  template <class... Args>                                                     \
  explicit class_name(Args &&...args)                                          \
      : pointer_to_copy{new (&storage) Type{std::forward<Args>(args)...}} {}   \
  template <class Tuple, std::size_t... I>                                     \
  explicit class_name(Tuple &&arg_tuple, std::index_sequence<I...> /*is*/)     \
      : pointer_to_copy{new (&storage) Type{                                   \
            std::get<I>(std::forward<Tuple>(arg_tuple))...}} {}                \
  template <class Tuple>                                                       \
  explicit class_name(Tuple &&arg_tuple)                                       \
      : class_name(std::forward<Tuple>(arg_tuple),                             \
                   std::make_index_sequence<                                   \
                       std::tuple_size_v<std::remove_reference_t<Tuple>>>{}) { \
  }
// ManagedStorage v1 uses aligned_storage_t and inplace new
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
// ManagedStorage v2 uses a C array and inplace new
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

// ManagedStorage v3 uses union and in-place new, DON'T USE THIS ONE
// explicit destructor is unreliable
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
// END MANAGEDSTORAGE CLASSES (to be able to hold a Type)

// START WRITE_API
// START object_stream_v1
template <typename Type>
[[nodiscard]] auto write_object_to_stream(const Type &object,
                                          std::ofstream &ofs_output_file)
    -> bool {
  ofs_output_file.write(reinterpret_cast<const char *>(&object),
                        sizeof(Type));  // NOLINT
  return ofs_output_file.good();
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
  std::array<char, N> output_buffer_of_bytes{};
  std::memcpy(output_buffer_of_bytes.data(), &object, N);
  return output_buffer_of_bytes;
}
// END object_buffer_v1_array
// START object_buffer_v1_vector
template <typename Type>
[[nodiscard]] auto write_object_to_buffer(const Type &object)
    -> std::vector<char> {
  std::vector<char> output_buffer_of_bytes(sizeof(Type));
  std::memcpy(output_buffer_of_bytes.data(), &object, sizeof(Type));
  return output_buffer_of_bytes;
}
// END object_buffer_v1_vector

// START stream_v1
template <typename Type>
auto write_vector_to_stream(const std::vector<Type> &container_of_type,
                            std::ofstream &ofs_output_file) -> bool {
  ofs_output_file.write(
      reinterpret_cast<const char *>(container_of_type.data()),  // NOLINT
      static_cast<long int>(sizeof(Type) * container_of_type.size()));
  return ofs_output_file.good();
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
    std::array<Type, N> &vector_input_data)
    -> std::array<char, N * sizeof(Type)> {
  std::array<char, N * sizeof(Type)> output_buffer_of_bytes{};
  std::memcpy(output_buffer_of_bytes.data(), vector_input_data.data(),
              N * sizeof(Type));
  return output_buffer_of_bytes;
}
// END buffer_v1_array
// START buffer_v1_vector
template <class Type>
[[nodiscard]] auto write_vector_to_buffer(std::vector<Type> &vector_input_data)
    -> std::vector<char> {
  const size_t vector_byte_size = vector_input_data.size() * sizeof(Type);
  std::vector<char> output_buffer_of_bytes(vector_byte_size);
  std::memcpy(output_buffer_of_bytes.data(), vector_input_data.data(),
              vector_byte_size);
  return output_buffer_of_bytes;
}
// END buffer_v1_vector

// START BYTEVECTORWITHCOUNTER
template <class ContainerOrViewType>
struct ByteContainerOrViewWithCounter {
  ContainerOrViewType byte_data{};
  std::optional<size_t> byte_counter{0};
  ByteContainerOrViewWithCounter() = default;
  template <class... Args>
  explicit ByteContainerOrViewWithCounter(Args &&...args)
      : byte_data(std::forward<Args>(args)...) {}

  /*
  ByteContainerOrViewWithCounter(ByteContainerOrViewWithCounter &_rhs)
      : byte_data(std::begin(_rhs.byte_data), std::end(_rhs.byte_data)),
        byte_counter(_rhs.byte_counter) {}

  ByteContainerOrViewWithCounter(ByteContainerOrViewWithCounter &&_rhs) noexcept
  : byte_data(std::move(_rhs.byte_data)), byte_counter(_rhs.byte_counter) {}*/

  [[nodiscard]] auto size() const { return byte_data.size(); }
  [[nodiscard]] auto size_remaining() const {
    return byte_counter ? size() - byte_counter.value() : 0;
  }

  void set_counter(size_t new_counter) { byte_counter = new_counter; }

  [[nodiscard]] auto would_it_be_full_if_so_invalidate(size_t size_to_advance)
      -> bool {
    if (size_to_advance > size_remaining()) {
      if (PICKLEJAR_ENABLE_VERBOSE_MODE) {
        PICKLEJAR_ASSERT(
            0,
            PICKLEJAR_RUNTIME_BYTEVECTORWITHCOUNTER_BYTE_COUNTER_INVALIDATED);
      }
      byte_counter.reset();
      return true;
    }
    return false;
  }

  [[nodiscard]] auto advance_counter(size_t size_to_advance) -> bool {
    if (would_it_be_full_if_so_invalidate(size_to_advance)) return false;
    byte_counter.value() += size_to_advance;
    return true;
  }

  auto write(const char *object_ptr, size_t object_size) -> bool {
    if (would_it_be_full_if_so_invalidate(object_size)) return false;
    std::memcpy(byte_data.data() + byte_counter.value(), object_ptr,
                object_size);
    byte_counter.value() += object_size;
    return true;
  }

  template <class Type>
  auto write(const Type &object, size_t object_size) -> bool {
    return write(reinterpret_cast<const char *>(&object), object_size);
  }

  template <class Type>
  auto write(const Type &object) -> bool {
    return write(object, sizeof(Type));
  }

  auto current_data_pos() {
    if (!byte_counter)
      assert(0);  // counter shouldn't have been invalidated at this point
    return byte_data.data() + long(byte_counter.value());
  }

  template <class PointerType>
  auto read(PointerType *destination_to_copy_to, const size_t size_to_read)
      -> bool {
    if (would_it_be_full_if_so_invalidate(size_to_read)) return false;
    // NOLINTNEXTLINE
    std::memcpy(reinterpret_cast<char *>(destination_to_copy_to),
                current_data_pos(), size_to_read);
    byte_counter.value() += size_to_read;
    return true;
  }

  template <class Type,
            class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
  [[nodiscard]] auto read(ManagedAlignedCopy &copy) -> ManagedAlignedCopy & {
    read(copy.get_pointer_to_copy(), sizeof(Type));
    return copy;
  }

  template <class Type,
            class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
  [[nodiscard]] auto read() -> std::optional<Type> {
    static_assert(
        std::is_trivially_copyable_v<Type>,
        "Object needs to be trivially copiable, you can use the "
        "read_object_from_buffer API if you want to copy non-trivial types.");
    ManagedAlignedCopy copy{};
    return *(read<Type>(copy)).get_pointer_to_copy();
  }
  auto begin() { return std::begin(byte_data); }
  auto end() { return std::end(byte_data); }
  friend auto begin(ByteContainerOrViewWithCounter &byte_vector_with_counter) {
    return byte_vector_with_counter.begin();
  }
  friend auto end(ByteContainerOrViewWithCounter &byte_vector_with_counter) {
    return byte_vector_with_counter.end();
  }

  auto current_iterator() {
    if (!byte_counter) return end();
    return begin() + long(byte_counter.value());
  }

  // return current_iterator + offset
  auto offset_iterator(size_t size_to_advance) {
    if (would_it_be_full_if_so_invalidate(size_to_advance)) return end();
    return current_iterator() + long(size_to_advance);
  }

  auto get_remaining_bytes_as_vector() -> std::vector<char> {
    return {current_iterator(), std::end(byte_data)};
  }

  auto get_remaining_bytes_as_span() -> std::span<char> {
    return {current_iterator(), size_remaining()};
  }

  [[nodiscard]] auto invalid() const -> bool {
    return !byte_counter.has_value();
  }
};

struct ByteSpanWithCounter : ByteContainerOrViewWithCounter<std::span<char>> {
  ByteSpanWithCounter(ByteSpanWithCounter &_rhs) = default;
  ByteSpanWithCounter(ByteSpanWithCounter &&_rhs) noexcept = default;
  explicit ByteSpanWithCounter(std::span<char> _byte_data)
      : ByteContainerOrViewWithCounter<std::span<char>>(std::begin(_byte_data),
                                                        _byte_data.size()) {}
  explicit ByteSpanWithCounter(auto _iterator, size_t _data_size)
      : ByteContainerOrViewWithCounter<std::span<char>>(_iterator, _data_size) {
  }
};

struct ByteVectorWithCounter
    : ByteContainerOrViewWithCounter<std::vector<char>> {
  ByteVectorWithCounter(ByteVectorWithCounter &_rhs) = default;
  ByteVectorWithCounter(ByteVectorWithCounter &&_rhs) noexcept = default;
  explicit ByteVectorWithCounter(size_t number_of_bytes)
      : ByteContainerOrViewWithCounter<std::vector<char>>{number_of_bytes} {}

  explicit ByteVectorWithCounter(auto begin_iterator, auto end_iterator)
      : ByteContainerOrViewWithCounter<std::vector<char>>{begin_iterator,
                                                          end_iterator} {}
  auto get_remaining_bytes() -> ByteVectorWithCounter {
    return ByteVectorWithCounter{current_iterator(), std::end(byte_data)};
  }
  auto get_remaining_bytes_as_span_with_counter() -> ByteSpanWithCounter {
    return ByteSpanWithCounter{current_iterator(), size_remaining()};
  }
};
// END BYTEVECTORWITHCOUNTER

// START buffer_v1
template <typename Type>
auto write_vector_to_buffer(const std::vector<Type> &container_of_type,
                            ByteVectorWithCounter &byte_vector_with_counter)
    -> bool {
  return byte_vector_with_counter.write(
      reinterpret_cast<const char *>(container_of_type.data()),  // NOLINT
      static_cast<long int>(sizeof(Type) * container_of_type.size()));
}
// END buffer_v1

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
// END READ_API_HELPERS

// START CONCEPTS
// only enable concepts if c++ > 17
#if ((defined(_MSVC_LANG) && _MSVC_LANG > 201703L) || __cplusplus > 201703L)
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
  { a.empty() } -> std::same_as<bool>;
};
template <typename C>
concept TriviallyCopiable = std::is_trivially_copyable_v<C>;

template <typename C>
concept CanBeCopiedEasily = std::is_standard_layout_v<C>;

template <typename C>
concept DefaultConstructible = std::is_default_constructible_v<C>;

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
  "lambda to fix non trivially copiable members. SEE DEEP COPY VERSIONING "    \
  "EXAMPLES "                                                                  \
  "versioning section in the readme file"

#define DEFAULTCONSTRUCTIBLE_MSG                                              \
  "PICKLEJAR_HELP: The object passed to picklejar is not default "            \
  "constructible. "                                                           \
  "You will need to pass a fourth parameter to picklejar with a lambda that " \
  "returns a tuple with the parameters to construct the object. SEE NON "     \
  "TRIVIAL EXAMPLES section in the readme file"

// START LAMBDA CONCEPTS
// Concept 1 return is a tuple and Type is constructible with it's elements
template <template <typename...> class Template, typename T>
struct is_specialization_of : std::false_type {};

template <template <typename...> class Template, typename... Args>
struct is_specialization_of<Template, Template<Args...>> : std::true_type {};

template <class Type, template <typename...> class Template, typename T>
struct is_constructible_with_elements_of_tuple : std::false_type {};
template <class Type, template <typename...> class Template, typename... Args>
struct is_constructible_with_elements_of_tuple<Type, Template,
                                               Template<Args...>>
    : std::is_constructible<Type, Args...> {};

template <typename ConstructorGeneratorLambda, typename Type>
concept PickleJarConstructorGeneratorLambdaRequirements =
    requires(ConstructorGeneratorLambda function, Type resulting_type) {
  requires is_specialization_of<
      std::tuple, std::invoke_result_t<ConstructorGeneratorLambda>>::value;
  requires is_constructible_with_elements_of_tuple<
      Type, std::tuple,
      std::invoke_result_t<ConstructorGeneratorLambda>>::value;
  {function()};
};

#define CONSTRUCTORGENERATORLAMBDAREQUIREMENTS_MSG                             \
  "PICKLEJAR_HELP: malformed 'constructor_generator_lambda' this lambda must " \
  "take no arguments and return a tuple with parameters that will be passed "  \
  "to construct the object"
// Concept 2 takes correct arguments and returns nothing
template <typename ManipulateBytesLambda, typename Type,
          typename ByteStorageMethod = std::array<char, sizeof(Type)>>
concept PickleJarManipulateBytesLambdaRequirements =
    requires(ManipulateBytesLambda function, Type blank_instance,
             ByteStorageMethod valid_bytes_blank_instance_copy,
             ByteStorageMethod bytes_from_file) {
  {
    function(blank_instance, valid_bytes_blank_instance_copy, bytes_from_file)
    } -> std::same_as<void>;
};
#define MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG                                  \
  "PICKLEJAR_HELP: malformed "                                                 \
  "'manipulate_bytes_from_file_before_writing_to_instance_lambda' this "       \
  "lambda must take: 1) 'a reference to an instance of the type you are "      \
  "trying to read', 2) 'a reference to a array<char, sizeof(Type)>, which "    \
  "contains a copy "                                                           \
  "of the first parameter bytes', 3) 'a reference to an array<char, "          \
  "sizeof(Type)>, which "                                                      \
  "contains the bytes read from the file for each object in the file' and no " \
  "return type"
// END LAMBDA CONCEPTS
// START DEEP COPY CONCEPTS
// Concept 1 takes correct arguments and returns true if write success
template <typename WriteElementLambda, typename BufferOrStreamObject,
          typename Type>
concept PickleJarWriteLambdaRequirements = requires(
    WriteElementLambda function, BufferOrStreamObject buffer_or_stream_object,
    Type templated_object, size_t object_size) {
  {
    function(buffer_or_stream_object, templated_object, object_size)
    } -> std::same_as<bool>;  // write_element_lambda parameter needs to return
                              // a true if write successful
};
// Concept 2 takes an object and returns a size_t
template <typename ElementSizeGetterLambda, typename Type>
concept PickleJarElementSizeGetterRequirements =
    requires(ElementSizeGetterLambda function, Type templated_object) {
  { function(templated_object) } -> std::same_as<size_t>;
};
// Concept 3 takes a byte buffer and returns true if successful
template <typename ByteBufferLambda>
concept PickleJarByteBufferLambdaRequirements = requires(
    ByteBufferLambda function, ByteVectorWithCounter byte_vector_with_counter) {
  { function(byte_vector_with_counter) } -> std::same_as<bool>;
};
// Concept 4 takes vector and byte_vector and returns true if successful
// insertion into vector
template <typename VectorInsertElementLambda, typename Container>
concept PickleJarVectorInsertElementLambdaRequirements =
    requires(VectorInsertElementLambda function, Container container,
             ByteVectorWithCounter byte_vector_with_counter) {
  { function(container, byte_vector_with_counter) } -> std::same_as<bool>;
};
// Concept 5 container has size
template <typename C>
concept ContainerDeepCopyReadRequirements = requires(C a) {
  { C() } -> std::same_as<C>;
  { a.size() } -> std::same_as<typename C::size_type>;
  { a.begin() } -> std::same_as<typename C::iterator>;
  { a.end() } -> std::same_as<typename C::iterator>;
  { a.empty() } -> std::same_as<bool>;
};
#define WRITELAMBDAREQUIREMENTS_MSG                                          \
  "PICKLEJAR_HELP: malformed 'write_element_lambda', make sure the "         \
  "lambda function takes the right parameters and returns true if write is " \
  "successful"

#define SIZEGETTERLAMBDAREQUIREMENTS_MSG                                    \
  "PICKLEJAR_HELP: malformed 'element_size_getter_lambda', make "           \
  "sure the lambda function takes the right element type as parameter and " \
  "returns size_t"

#define BYTEBUFFERLAMBDAREQUIREMENTS_MSG                                       \
  "PICKLEJAR_HELP: malformed 'byte_buffer_lambda', make "                      \
  "sure the lambda function takes a ByteVectorWithCounter as a parameter and " \
  "returns true if reading operation was successful"

#define VECTORINSERTELEMENTLAMBDAREQUIREMENTS_MSG                   \
  "PICKLEJAR_HELP: malformed 'vector_insert_element_lambda', make " \
  "sure the lambda function takes a BufferOrStreamObject, and a "   \
  "ByteVectorWithCounter as a parameter and "                       \
  "returns true if reading operation was successful"
#define CONTAINERDEEPCOPYREADREQUIREMENTS_MSG                               \
  "PICKLEJAR_HELP: You need to pass a container<Type> as first param that " \
  "has .size() and is iterable Ex: std::vector or std::array"

#define PICKLEJAR_CONCEPT(conditional, message) \
  static_assert(conditional, message)
#else
#define PICKLEJAR_CONCEPT(conditional, message)
#endif
// END DEEP COPY CONCEPTS

// CONCEPTS THAT ARE USE IN CONSTEXPR STATEMENTS
template <typename C>
concept ContainerHasReserve = requires(C a) {
  { C() } -> std::same_as<C>;
  { a.size() } -> std::same_as<typename C::size_type>;
  { a.reserve() } -> std::same_as<void>;
  { a.empty() } -> std::same_as<bool>;
};

template <typename C>
concept SequentialContainerOfChar = requires(C a) {
  requires std::same_as<typename C::value_type, char>;
  { a.size() } -> std::same_as<typename C::size_type>;
  { a.data() } -> std::same_as<typename C::value_type *>;
  { a.empty() } -> std::same_as<bool>;
};

template <typename C>
concept PickleJarValidByteContainerOrViewType = requires(C a) {
  requires std::same_as<C, ByteVectorWithCounter> ||
      std::same_as<C, ByteSpanWithCounter>;
};

#define VALIDBYTECONTAINERORVIEWTYPE_MSG                                  \
  "PICKLEJAR_HELP: You need to pass either a ByteVectorWithCounter or a " \
  "ByteSpanWithCounter"

// END CONCEPTS

// START READ_API
// START object_stream_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
[[nodiscard]] auto operation_specific_read_object_from_stream(
    ManagedAlignedCopy &copy, std::ifstream &ifstream_input_file)
    -> ManagedAlignedCopy & {
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
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
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
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
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
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
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
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
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  PICKLEJAR_CONCEPT((ContainerHasPushBack<Container, Type>),
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
            ifstream_input_file, size_t(file_size)))
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
    return PICKLEJAR_MAKE_OPTIONAL(vector_input_data);
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
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  PICKLEJAR_CONCEPT((ContainerHasPushBack<Container, Type>),
                    CONTAINERWITHHASPUSHBACK_MSG);
  Container vector_input_data;
  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{read_vector_from_stream<Type, ManagedAlignedCopy>(
      vector_input_data, ifstream_input_file)};
  if (ifstream_close_and_check_is_invalid(ifstream_input_file) or !result) {
    return {};
  }
  return std::make_optional(RETURN_RESULT_FROM_FILE);
}
// END file_v1 uses stream_v1

// START object_stream_v2_nochecks
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ManipulateBytesLambda>
auto operation_specific_read_object_from_stream(
    ManagedAlignedCopy &copy, std::ifstream &ifstream_input_file,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> ManagedAlignedCopy & {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  std::array<char, sizeof(Type)> valid_bytes_blank_instance_copy{};
  std::memcpy(valid_bytes_blank_instance_copy.data(),
              copy.get_pointer_to_copy(), sizeof(Type));
  std::array<char, sizeof(Type)> bytes_from_file{};
  ifstream_input_file.read(bytes_from_file.data(), sizeof(Type));
  manipulate_bytes_from_file_before_writing_to_instance_lambda(
      *copy.get_pointer_to_copy(), valid_bytes_blank_instance_copy,
      bytes_from_file);
  return copy;
}
// END object_stream_v2_nochecks

// START object_stream_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ManipulateBytesLambda>
auto read_object_from_stream(
    std::ifstream &ifstream_input_file,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> std::optional<Type> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  ManagedAlignedCopy copy{};
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{
      operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
          copy, ifstream_input_file,
          manipulate_bytes_from_file_before_writing_to_instance_lambda)
          .get_pointer_to_copy()};
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  return std::make_optional(*result);
}
// END object_stream_v2

// START object_file_v2_nochecks uses object_stream_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ManipulateBytesLambda>
auto operation_specific_read_object_from_file(
    ManagedAlignedCopy &copy, const std::string file_name,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> ManagedAlignedCopy & {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  return operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
      copy, ifstream_input_file,
      manipulate_bytes_from_file_before_writing_to_instance_lambda);
}
// END object_file_v2_nochecks uses object_stream_v2
// START object_file_v2 uses object_stream_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ManipulateBytesLambda>
auto read_object_from_file(
    ManagedAlignedCopy &copy, const std::string file_name,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> std::optional<ManagedAlignedCopy *> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{
      &operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
          copy, ifstream_input_file,
          manipulate_bytes_from_file_before_writing_to_instance_lambda)};
  if (ifstream_close_and_check_is_invalid(ifstream_input_file)) {
    return {};
  }
  return std::make_optional(result);
}
// END object_file_v2 uses object_stream_v2
// START object_file_v2_copy uses object_file_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ManipulateBytesLambda>
auto read_object_from_file(
    const std::string file_name,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> std::optional<Type> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  ManagedAlignedCopy copy{};
  auto lower_level_return{read_object_from_file<Type, ManagedAlignedCopy>(
      copy, file_name,
      manipulate_bytes_from_file_before_writing_to_instance_lambda)};
  if (lower_level_return)
    return std::make_optional(
        *(lower_level_return.value())->get_pointer_to_copy());
  return {};
}
// END object_file_v2_copy uses object_file_v2

// START object_buffer_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ManipulateBytesLambda>
auto operation_specific_read_object_from_buffer(
    ManagedAlignedCopy &copy, ByteVectorWithCounter &buffer_with_input_bytes,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> ManagedAlignedCopy & {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);

  std::array<char, sizeof(Type)> valid_bytes_blank_instance_copy{};
  std::memcpy(valid_bytes_blank_instance_copy.data(),
              copy.get_pointer_to_copy(), sizeof(Type));
  std::array<char, sizeof(Type)>
      bytes_from_file{};  // TODO(tom): this may be unnecessary we could just
                          // pass buffer_with_input_bytes with offset and size?
  std::memcpy(bytes_from_file.data(),
              buffer_with_input_bytes.current_data_pos(), sizeof(Type));
  buffer_with_input_bytes.byte_counter.value() += sizeof(Type);
  manipulate_bytes_from_file_before_writing_to_instance_lambda(
      *copy.get_pointer_to_copy(), valid_bytes_blank_instance_copy,
      bytes_from_file);
  return copy;
}
// END object_buffer_v2
// START object_buffer_v2_copy uses object_buffer_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ManipulateBytesLambda, class ByteContainerOrViewType>
constexpr auto read_object_from_buffer(
    ByteContainerOrViewType &buffer_with_input_bytes,
    ManipulateBytesLambda &&
        manipulate_bytes_from_file_before_writing_to_instance_lambda) -> Type {
  PICKLEJAR_CONCEPT(
      PickleJarValidByteContainerOrViewType<ByteContainerOrViewType>,
      VALIDBYTECONTAINERORVIEWTYPE_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  ManagedAlignedCopy copy{};
  return *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
               copy, buffer_with_input_bytes,
               manipulate_bytes_from_file_before_writing_to_instance_lambda))
              .get_pointer_to_copy();
}
// END object_buffer_v2_copy uses object_buffer_v2

// START object_stream_v3 uses object_stream_v2
// this returns a copy, object_stream_v2 be more efficient because it takes
// and returns a reference therefore only generating a move
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ConstructorGeneratorLambda, class ManipulateBytesLambda>
[[nodiscard]] auto read_object_from_stream(
    std::ifstream &ifstream_input_file,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda,
    ConstructorGeneratorLambda &&constructor_generator_lambda)
    -> std::optional<Type> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT((PickleJarConstructorGeneratorLambdaRequirements<
                        ConstructorGeneratorLambda, Type>),
                    CONSTRUCTORGENERATORLAMBDAREQUIREMENTS_MSG);
  ManagedAlignedCopy copy{constructor_generator_lambda()};
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{std::make_optional(
      *(operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
            copy, ifstream_input_file,
            manipulate_bytes_from_file_before_writing_to_instance_lambda))
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
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ConstructorGeneratorLambda, class ManipulateBytesLambda>
[[nodiscard]] auto read_object_from_file(
    const std::string file_name,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda,
    ConstructorGeneratorLambda &&constructor_generator_lambda)
    -> std::optional<Type> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT((PickleJarConstructorGeneratorLambdaRequirements<
                        ConstructorGeneratorLambda, Type>),
                    CONSTRUCTORGENERATORLAMBDAREQUIREMENTS_MSG);
  ManagedAlignedCopy copy{constructor_generator_lambda()};
  auto lower_level_return{read_object_from_file<Type, ManagedAlignedCopy>(
      copy, file_name,
      manipulate_bytes_from_file_before_writing_to_instance_lambda)};
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
          class ConstructorGeneratorLambda, class ManipulateBytesLambda,
          class ByteContainerOrViewType>
[[nodiscard]] constexpr auto read_object_from_buffer(
    ByteContainerOrViewType &buffer_with_input_bytes,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda,
    ConstructorGeneratorLambda &&constructor_generator_lambda) -> Type {
  PICKLEJAR_CONCEPT(
      PickleJarValidByteContainerOrViewType<ByteContainerOrViewType>,
      VALIDBYTECONTAINERORVIEWTYPE_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT((PickleJarConstructorGeneratorLambdaRequirements<
                        ConstructorGeneratorLambda, Type>),
                    CONSTRUCTORGENERATORLAMBDAREQUIREMENTS_MSG);
  ManagedAlignedCopy copy{constructor_generator_lambda()};
  return *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
               copy, buffer_with_input_bytes,
               manipulate_bytes_from_file_before_writing_to_instance_lambda))
              .get_pointer_to_copy();
}
// END object_buffer_v3 uses object_buffer_v2
namespace util {
template <size_t N>
void preserve_blank_instance_member(
    size_t blank_instance_member_offset, size_t blank_instance_member_size,
    std::array<char, N> &valid_bytes_blank_instance_copy,
    std::array<char, N> &bytes_from_file) {
  // copy valid std::string bytes to the bytes we read from the file
  std::memcpy(
      bytes_from_file.data() + blank_instance_member_offset,
      valid_bytes_blank_instance_copy.data() + blank_instance_member_offset,
      blank_instance_member_size);
}
template <size_t N, class Type>
void copy_new_bytes_to_instance(std::array<char, N> &bytes_to_copy_to_instance,
                                Type &blank_instance, size_t size_of_object) {
  // copy our read bytes to our blank copy
  std::memcpy(&blank_instance, bytes_to_copy_to_instance.data(),
              size_of_object);
}
}  // namespace util

// START object_buffer_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
[[nodiscard]] auto operation_specific_read_object_from_buffer(
    ManagedAlignedCopy &copy, ByteVectorWithCounter &buffer_with_input_bytes)
    -> ManagedAlignedCopy & {
  return buffer_with_input_bytes.read<Type>(copy);
}
// END object_buffer_v1

// START buffer_v1 uses object_buffer_v1
// BUFFER VERSION taken from OPERATION VERSION
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container, class ByteContainerOrViewType>
[[nodiscard]] constexpr auto read_vector_from_buffer(
    Container &vector_input_data,
    ByteContainerOrViewType &buffer_with_input_bytes)
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(
      PickleJarValidByteContainerOrViewType<ByteContainerOrViewType>,
      VALIDBYTECONTAINERORVIEWTYPE_MSG);
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  PICKLEJAR_CONCEPT((ContainerHasPushBack<Container, Type>),
                    CONTAINERWITHHASPUSHBACK_MSG);
  auto file_size = buffer_with_input_bytes.size();
  auto initial_vector_size = vector_input_data.size();
  // std::puts(("file_length: " + std::to_string(file_size)).c_str());
  // std::puts(("sizeof Type: " + std::to_string(sizeof(Type))).c_str());
  if (file_size < 1) {
    return {};
  }
  // create a REFERENCE so we don't have to type this twice
  size_t &bytes_read_so_far = buffer_with_input_bytes.byte_counter.value();
  while (bytes_read_so_far < file_size) {
    if (bytes_read_so_far + sizeof(Type) > file_size) break;
    ManagedAlignedCopy copy{};
    vector_input_data.push_back(std::move(
        *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
              copy, buffer_with_input_bytes))
             .get_pointer_to_copy()));
  }
  if (vector_input_data.size() > initial_vector_size) {
    return PICKLEJAR_MAKE_OPTIONAL(vector_input_data);
  }
  return {};
}
// END buffer_v1 uses object_buffer_v1
// START buffer_v3 uses object_buffer_v2
// BUFFER VERSION taken from OPERATION VERSION
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container, class ConstructorGeneratorLambda,
          class ManipulateBytesLambda, class ByteContainerOrViewType>
[[nodiscard]] constexpr auto read_vector_from_buffer(
    Container &vector_input_data,
    ByteContainerOrViewType &buffer_with_input_bytes,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda,
    ConstructorGeneratorLambda &&constructor_generator_lambda)
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(
      PickleJarValidByteContainerOrViewType<ByteContainerOrViewType>,
      VALIDBYTECONTAINERORVIEWTYPE_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT((PickleJarConstructorGeneratorLambdaRequirements<
                        ConstructorGeneratorLambda, Type>),
                    CONSTRUCTORGENERATORLAMBDAREQUIREMENTS_MSG);
  auto file_size = buffer_with_input_bytes.size();
  auto initial_vector_size = vector_input_data.size();
  // std::puts(("file_length: " + std::to_string(file_size)).c_str());
  // std::puts(("sizeof Type: " + std::to_string(sizeof(Type))).c_str());
  if (file_size < 1) {
    return {};
  }
  // create a REFERENCE so we don't have to type this twice
  size_t &bytes_read_so_far = buffer_with_input_bytes.byte_counter.value();
  while (bytes_read_so_far < file_size) {
    // std::puts(
    //    ("bytes_read_so_far: " + std::to_string(bytes_read_so_far)).c_str());
    if (bytes_read_so_far + sizeof(Type) > file_size) break;
    // START OPERATION VERSION
    ManagedAlignedCopy copy{constructor_generator_lambda()};

    // END OPERATION VERSION
    vector_input_data.push_back(std::move(
        *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
              copy, buffer_with_input_bytes,
              manipulate_bytes_from_file_before_writing_to_instance_lambda))
             .get_pointer_to_copy()));
  }
  if (vector_input_data.size() > initial_vector_size) {
    return PICKLEJAR_MAKE_OPTIONAL(vector_input_data);
  }
  return {};
}
// END buffer_v3 uses object_buffer_v2
// START stream_v3 uses object_stream_v2
// OPERATION VERSION
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container, class ConstructorGeneratorLambda,
          class ManipulateBytesLambda>
[[nodiscard]] auto read_vector_from_stream(
    Container &vector_input_data, std::ifstream &ifstream_input_file,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda,
    ConstructorGeneratorLambda &&constructor_generator_lambda)
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT((PickleJarConstructorGeneratorLambdaRequirements<
                        ConstructorGeneratorLambda, Type>),
                    CONSTRUCTORGENERATORLAMBDAREQUIREMENTS_MSG);
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
            ifstream_input_file, size_t(file_size)))
      break;
    ManagedAlignedCopy copy{constructor_generator_lambda()};
    vector_input_data.push_back(std::move(
        *(operation_specific_read_object_from_stream<Type, ManagedAlignedCopy>(
              copy, ifstream_input_file,
              manipulate_bytes_from_file_before_writing_to_instance_lambda))
             .get_pointer_to_copy()));
  }
  if (vector_input_data.size() > initial_vector_size) {
    return PICKLEJAR_MAKE_OPTIONAL(vector_input_data);
  }
  return {};
}
// END stream_v3 uses object_stream_v2
// START stream_v2 uses stream_v3
constexpr auto return_empty_tuple = []() { return std::tuple(); };

template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container, class ManipulateBytesLambda>
[[nodiscard]] auto read_vector_from_stream(
    Container &vector_input_data, std::ifstream &ifstream_input_file,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  return read_vector_from_stream<Type, ManagedAlignedCopy>(
      vector_input_data, ifstream_input_file,
      manipulate_bytes_from_file_before_writing_to_instance_lambda,
      return_empty_tuple);
}
// END stream_v2 uses stream_v3
// START buffer_v2 uses buffer_v3
template <
    class Type, class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
    class Container, class ManipulateBytesLambda, class ByteContainerOrViewType>
[[nodiscard]] constexpr auto read_vector_from_buffer(
    Container &vector_input_data,
    ByteContainerOrViewType &buffer_with_input_bytes,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(
      PickleJarValidByteContainerOrViewType<ByteContainerOrViewType>,
      VALIDBYTECONTAINERORVIEWTYPE_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  return read_vector_from_buffer<Type, ManagedAlignedCopy>(
      vector_input_data, buffer_with_input_bytes,
      manipulate_bytes_from_file_before_writing_to_instance_lambda,
      return_empty_tuple);
}
// END buffer_v2 uses buffer_v3
// START file_v2 uses stream_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container = std::vector<Type>, class ManipulateBytesLambda>
[[nodiscard]] auto read_vector_from_file(
    const std::string file_name,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> std::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  Container vector_input_data;

  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{read_vector_from_stream<Type, ManagedAlignedCopy>(
      vector_input_data, ifstream_input_file,
      manipulate_bytes_from_file_before_writing_to_instance_lambda)};
  if (ifstream_close_and_check_is_invalid(ifstream_input_file) or !result) {
    return {};
  }
  return std::make_optional(RETURN_RESULT_FROM_FILE);
}
// END file_v2 uses stream_v2
// START file_v3 uses stream_v3
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container = std::vector<Type>, class ConstructorGeneratorLambda,
          class ManipulateBytesLambda>
[[nodiscard]] auto read_vector_from_file(
    const std::string file_name,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda,
    ConstructorGeneratorLambda &&constructor_generator_lambda)
    -> std::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT((PickleJarConstructorGeneratorLambdaRequirements<
                        ConstructorGeneratorLambda, Type>),
                    CONSTRUCTORGENERATORLAMBDAREQUIREMENTS_MSG);
  Container vector_input_data;
  std::ifstream ifstream_input_file(file_name, std::ios::in | std::ios::binary);
  if (ifstream_is_invalid(ifstream_input_file)) {
    return {};
  }
  auto result{read_vector_from_stream<Type, ManagedAlignedCopy>(
      vector_input_data, ifstream_input_file,
      manipulate_bytes_from_file_before_writing_to_instance_lambda,
      constructor_generator_lambda)};
  if (ifstream_close_and_check_is_invalid(ifstream_input_file) or !result) {
    return {};
  }
  return std::make_optional(RETURN_RESULT_FROM_FILE);
}
// END file_v3 uses stream_v3
// END READ_API

// DEEP COPY FUNCTIONS
template <class BufferOrStreamObject>
constexpr auto get_buffer_or_stream_byte_counter(
    BufferOrStreamObject &buffer_or_stream_object) -> size_t {
  if constexpr (std::same_as<BufferOrStreamObject, std::ofstream>) {
    return size_t(buffer_or_stream_object.tellp());
  } else if constexpr (std::same_as<BufferOrStreamObject, std::ifstream>) {
    return size_t(buffer_or_stream_object.tellg());
  } else {
    return buffer_or_stream_object.byte_counter.value();
  }
}

template <size_t Version = 0, class BufferOrStreamObject,
          bool WriteSizeFunction(const size_t &, BufferOrStreamObject &) =
              picklejar::write_object_to_stream<size_t>,
          class Type, class WriteElementLambda>
auto write_object_deep_copy(const Type &object, const size_t object_size,
                            BufferOrStreamObject &buffer_or_stream_object,
                            WriteElementLambda &&write_element_lambda) -> bool {
  PICKLEJAR_CONCEPT(
      (PickleJarWriteLambdaRequirements<WriteElementLambda,
                                        BufferOrStreamObject, Type>),
      WRITELAMBDAREQUIREMENTS_MSG);
  if constexpr (Version > 0) {
    if (!WriteSizeFunction(Version, buffer_or_stream_object)) return false;
  }
  if (WriteSizeFunction(object_size, buffer_or_stream_object)) {
    size_t total_size_written_calculation =
        get_buffer_or_stream_byte_counter(buffer_or_stream_object);

    bool return_value = write_element_lambda(buffer_or_stream_object, object,
                                             object_size);  // NOLINT
    total_size_written_calculation =
        get_buffer_or_stream_byte_counter(buffer_or_stream_object) -
        total_size_written_calculation;
    // clang-format off
    PICKLEJAR_ASSERT(total_size_written_calculation == object_size,
          "PICKLEJAR_RUNTIME_HELP: The size returned from the "
	  "'element_size_getter_lambda("<<type_name<Type>()<<")' is ("
	  << object_size <<") and the "
          "size written (" << total_size_written_calculation <<") from the"
	  "'write_element_lambda' does NOT match.\n"
          "Double check you are correctly returning the total size to be "
          "written for each object in the 'element_size_getter_lambda' and "
          "also that you are writting that same amount of bytes in the "
          "'write_element_lambda'");
    // clang-format on
    return return_value;
  }
  return false;
}

template <size_t Version = 0, class BufferOrStreamObject,
          bool WriteSizeFunction(const size_t &, BufferOrStreamObject &) =
              picklejar::write_object_to_stream<size_t>,
          class Container, class Type = typename Container::value_type,
          class WriteElementLambda, class ElementSizeGetterLambda>
auto write_vector_deep_copy(
    const Container &vector_input_data,
    BufferOrStreamObject &buffer_or_stream_object,
    ElementSizeGetterLambda &&element_size_getter_lambda,
    WriteElementLambda &&write_element_lambda) -> bool {
  PICKLEJAR_CONCEPT(ContainerDeepCopyReadRequirements<Container>,
                    CONTAINERDEEPCOPYREADREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarWriteLambdaRequirements<WriteElementLambda,
                                        BufferOrStreamObject, Type>),
      WRITELAMBDAREQUIREMENTS_MSG);

  PICKLEJAR_CONCEPT(
      (PickleJarElementSizeGetterRequirements<ElementSizeGetterLambda, Type>),
      SIZEGETTERLAMBDAREQUIREMENTS_MSG);

  if (vector_input_data.empty()) return false;
  if constexpr (Version > 0) {
    if (!WriteSizeFunction(Version, buffer_or_stream_object)) return false;
  }
  if (WriteSizeFunction(vector_input_data.size(), buffer_or_stream_object)) {
    for (const Type &object : vector_input_data) {
      size_t object_size{element_size_getter_lambda(object)};
      // for each element we write the size of the object first
      if (!write_object_deep_copy<0, BufferOrStreamObject, WriteSizeFunction>(
              object, object_size, buffer_or_stream_object,
              write_element_lambda)) {
        return false;
      }
    }
    return true;
  }
  return false;
}

template <class PointerType>
auto basic_stream_read(std::ifstream &ifstream_input_file,
                       PointerType *destination_to_copy_to,
                       const size_t size_to_read) -> bool {
  ifstream_input_file.read(
      reinterpret_cast<char *>(destination_to_copy_to),  // NOLINT
      std::streamsize(size_to_read));
  return ifstream_input_file.good();
};

template <size_t Version = 0, class BufferOrStreamObject,
          std::optional<size_t> ReadSizeFunction(BufferOrStreamObject &) =
              picklejar::read_object_from_stream<size_t>,
          bool ReadBufferOrStreamFunction(BufferOrStreamObject &, char *,
                                          const size_t) =
              picklejar::basic_stream_read,
          class ByteBufferLambda>
auto read_object_deep_copy(BufferOrStreamObject &buffer_or_stream_object,
                           ByteBufferLambda &&byte_buffer_lambda) -> bool {
  PICKLEJAR_CONCEPT((PickleJarByteBufferLambdaRequirements<ByteBufferLambda>),
                    BYTEBUFFERLAMBDAREQUIREMENTS_MSG);

  if constexpr (Version > 0) {
    if (auto optional_version = ReadSizeFunction(buffer_or_stream_object);
        !optional_version or optional_version.value() != Version) {
      if (PICKLEJAR_ENABLE_VERBOSE_MODE and optional_version) {
        PICKLEJAR_MESSAGE(optional_version.value() == Version,
                          PICKLEJAR_RUNTIME_READ_VERSION_MISSMATCH);
      }
      return false;
    }
  }

  if (auto optional_size = ReadSizeFunction(buffer_or_stream_object)) {
    // if we got the size of our object in optional_size.value()
    // we create a vector of char and we read the stream into it
    ByteVectorWithCounter byte_buffer(optional_size.value());
    if (ReadBufferOrStreamFunction(buffer_or_stream_object,
                                   byte_buffer.byte_data.data(),
                                   optional_size.value())) {
      // if read is sucessful we create the object using it's byte_buffer
      // bytes
      bool return_value = byte_buffer_lambda(byte_buffer);
      if (return_value &&
          optional_size.value() != byte_buffer.byte_counter.value()) {
        PICKLEJAR_ASSERT(
            optional_size.value() == byte_buffer.byte_counter.value(),
            PICKLEJAR_RUNTIME_READSIZE_MISSMATCH);
      }
      if (PICKLEJAR_ENABLE_VERBOSE_MODE && !return_value) {
        PICKLEJAR_MESSAGE(
            optional_size.value() == byte_buffer.byte_counter.value(),
            PICKLEJAR_RUNTIME_READSIZE_MISSMATCH);
      }
      return return_value;
    }
  }
  return false;
}

template <size_t Version = 0, class BufferOrStreamObject,
          std::optional<size_t> ReadSizeFunction(BufferOrStreamObject &) =
              picklejar::read_object_from_stream<size_t>,
          bool ReadBufferOrStreamFunction(BufferOrStreamObject &, char *,
                                          const size_t) =
              picklejar::basic_stream_read,
          class Container, class VectorInsertElementLambda>
auto read_vector_deep_copy(
    Container &result, BufferOrStreamObject &buffer_or_stream_object,
    VectorInsertElementLambda &&vector_insert_element_lambda)
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarVectorInsertElementLambdaRequirements<VectorInsertElementLambda,
                                                      Container>),
      VECTORINSERTELEMENTLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(ContainerDeepCopyReadRequirements<Container>,
                    CONTAINERDEEPCOPYREADREQUIREMENTS_MSG);

  size_t result_initial_size{result.size()};
  if constexpr (Version > 0) {
    if (auto optional_version = ReadSizeFunction(buffer_or_stream_object);
        !optional_version or optional_version.value() != Version) {
      if (PICKLEJAR_ENABLE_VERBOSE_MODE and optional_version) {
        PICKLEJAR_MESSAGE(optional_version.value() == Version,
                          PICKLEJAR_RUNTIME_READ_VERSION_MISSMATCH);
      }
      return {};
    }
  }
  if (auto optional_size = ReadSizeFunction(buffer_or_stream_object)) {
    if constexpr (ContainerHasReserve<Container>) {
      result.reserve(optional_size.value());
    }
    for (size_t i{0}; i < optional_size.value(); ++i) {
      if (!read_object_deep_copy<0, BufferOrStreamObject, ReadSizeFunction,
                                 ReadBufferOrStreamFunction>(
              buffer_or_stream_object, [&](ByteVectorWithCounter &byte_buffer) {
                return vector_insert_element_lambda(result, byte_buffer);
              }))
        return {};
    }
  }
  if (result.size() > result_initial_size) {
    return PICKLEJAR_MAKE_OPTIONAL(result);
  }
  return {};
}

template <size_t Version = 0, class Container, class WriteElementLambda,
          class ElementSizeGetterLambda,
          typename Type = typename Container::value_type>
auto deep_copy_vector_to_stream(
    const Container &vector_input_data, std::ofstream &ofs_output_file,
    ElementSizeGetterLambda &&element_size_getter_lambda,
    WriteElementLambda &&write_element_lambda) -> bool {
  PICKLEJAR_CONCEPT(ContainerDeepCopyReadRequirements<Container>,
                    CONTAINERDEEPCOPYREADREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT((PickleJarWriteLambdaRequirements<WriteElementLambda,
                                                      std::ofstream, Type>),
                    WRITELAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarElementSizeGetterRequirements<ElementSizeGetterLambda, Type>),
      SIZEGETTERLAMBDAREQUIREMENTS_MSG);
  return write_vector_deep_copy<Version>(vector_input_data, ofs_output_file,
                                         element_size_getter_lambda,
                                         write_element_lambda);
}

template <size_t Version = 0, class BufferOrStreamObject,
          bool WriteSizeFunction(const size_t &, BufferOrStreamObject &) =
              picklejar::write_object_to_stream<size_t>,
          class Type, class WriteElementLambda>
auto deep_copy_object_to_stream(const Type &object, const size_t object_size,
                                BufferOrStreamObject &buffer_or_stream_object,
                                WriteElementLambda &&write_element_lambda)
    -> bool {
  PICKLEJAR_CONCEPT(
      (PickleJarWriteLambdaRequirements<WriteElementLambda,
                                        BufferOrStreamObject, Type>),
      WRITELAMBDAREQUIREMENTS_MSG);
  return write_object_deep_copy<Version>(
      object, object_size, buffer_or_stream_object, write_element_lambda);
}

template <size_t Version = 0, class Container, class WriteElementLambda,
          class ElementSizeGetterLambda,
          typename Type = typename Container::value_type>
auto deep_copy_vector_to_file(
    const Container &vector_input_data, const std::string file_name,
    ElementSizeGetterLambda &&element_size_getter_lambda,
    WriteElementLambda &&write_element_lambda) -> bool {
  PICKLEJAR_CONCEPT(ContainerDeepCopyReadRequirements<Container>,
                    CONTAINERDEEPCOPYREADREQUIREMENTS_MSG);

  PICKLEJAR_CONCEPT((PickleJarWriteLambdaRequirements<WriteElementLambda,
                                                      std::ofstream, Type>),
                    WRITELAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarElementSizeGetterRequirements<ElementSizeGetterLambda, Type>),
      SIZEGETTERLAMBDAREQUIREMENTS_MSG);
  std::ofstream ofs_output_file(file_name);
  return write_vector_deep_copy<Version>(vector_input_data, ofs_output_file,
                                         element_size_getter_lambda,
                                         write_element_lambda);
}

template <size_t Version = 0, class BufferOrStreamObject,
          bool WriteSizeFunction(const size_t &, BufferOrStreamObject &) =
              picklejar::write_object_to_stream<size_t>,
          class Type, class WriteElementLambda>
auto deep_copy_object_to_file(const Type &object, const size_t object_size,
                              const std::string file_name,
                              WriteElementLambda &&write_element_lambda)
    -> bool {
  PICKLEJAR_CONCEPT(
      (PickleJarWriteLambdaRequirements<WriteElementLambda,
                                        BufferOrStreamObject, Type>),
      WRITELAMBDAREQUIREMENTS_MSG);
  std::ofstream ofs_output_file(file_name);
  return write_object_deep_copy<Version>(object, object_size, ofs_output_file,
                                         write_element_lambda);
}

template <size_t Version = 0, class Container,
          typename Type = typename Container::value_type,
          class VectorInsertElementLambda>
auto deep_read_vector_from_stream(
    Container &result, std::ifstream &ifs_input_file,
    VectorInsertElementLambda &&vector_insert_element_lambda)
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarVectorInsertElementLambdaRequirements<VectorInsertElementLambda,
                                                      Container>),
      VECTORINSERTELEMENTLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(ContainerDeepCopyReadRequirements<Container>,
                    CONTAINERDEEPCOPYREADREQUIREMENTS_MSG);

  return read_vector_deep_copy<Version>(result, ifs_input_file,
                                        vector_insert_element_lambda);
}

template <size_t Version = 0, class BufferOrStreamObject,
          std::optional<size_t> ReadSizeFunction(BufferOrStreamObject &) =
              picklejar::read_object_from_stream<size_t>,
          bool ReadBufferOrStreamFunction(BufferOrStreamObject &, char *,
                                          const size_t) =
              picklejar::basic_stream_read,
          class ByteBufferLambda>
auto deep_read_object_to_stream(BufferOrStreamObject &buffer_or_stream_object,
                                ByteBufferLambda &&byte_buffer_lambda) -> bool {
  PICKLEJAR_CONCEPT((PickleJarByteBufferLambdaRequirements<ByteBufferLambda>),
                    BYTEBUFFERLAMBDAREQUIREMENTS_MSG);
  return read_object_deep_copy<Version>(buffer_or_stream_object,
                                        byte_buffer_lambda);
}

template <size_t Version = 0, class Container,
          typename Type = typename Container::value_type,
          class VectorInsertElementLambda>
auto deep_read_vector_from_file(
    Container &result, const std::string file_name,
    VectorInsertElementLambda &&vector_insert_element_lambda)
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarVectorInsertElementLambdaRequirements<VectorInsertElementLambda,
                                                      Container>),
      VECTORINSERTELEMENTLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(ContainerDeepCopyReadRequirements<Container>,
                    CONTAINERDEEPCOPYREADREQUIREMENTS_MSG);

  std::ifstream ifs_input_file(file_name);
  return read_vector_deep_copy<Version>(result, ifs_input_file,
                                        vector_insert_element_lambda);
}
template <size_t Version = 0, class BufferOrStreamObject,
          std::optional<size_t> ReadSizeFunction(BufferOrStreamObject &) =
              picklejar::read_object_from_stream<size_t>,
          bool ReadBufferOrStreamFunction(BufferOrStreamObject &, char *,
                                          const size_t) =
              picklejar::basic_stream_read,
          class ByteBufferLambda>
auto deep_read_object_from_file(const std::string file_name,
                                ByteBufferLambda &&byte_buffer_lambda) -> bool {
  PICKLEJAR_CONCEPT((PickleJarByteBufferLambdaRequirements<ByteBufferLambda>),
                    BYTEBUFFERLAMBDAREQUIREMENTS_MSG);
  std::ifstream ifs_input_file(file_name);
  return read_object_deep_copy<Version>(ifs_input_file, byte_buffer_lambda);
}

template <class PointerType>
auto basic_buffer_read(ByteVectorWithCounter &vector_byte_buffer,
                       PointerType *destination_to_copy_to,
                       const size_t size_to_read) -> bool {
  return vector_byte_buffer.read(destination_to_copy_to, size_to_read);
};

template <typename Type>
[[nodiscard]] auto write_object_to_buffer(
    const Type &object, ByteVectorWithCounter &output_buffer_of_bytes) -> bool {
  output_buffer_of_bytes.write(object);
  return true;
}

template <size_t Version = 0, class Container, class WriteElementLambda,
          class ElementSizeGetterLambda,
          typename Type = typename Container::value_type>
auto deep_copy_vector_to_buffer(
    const Container &vector_input_data,
    ElementSizeGetterLambda &&element_size_getter_lambda,
    WriteElementLambda &&write_element_lambda)
    -> std::optional<ByteVectorWithCounter> {
  PICKLEJAR_CONCEPT(ContainerDeepCopyReadRequirements<Container>,
                    CONTAINERDEEPCOPYREADREQUIREMENTS_MSG);

  PICKLEJAR_CONCEPT(
      (PickleJarWriteLambdaRequirements<WriteElementLambda,
                                        ByteVectorWithCounter, Type>),
      WRITELAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarElementSizeGetterRequirements<ElementSizeGetterLambda, Type>),
      SIZEGETTERLAMBDAREQUIREMENTS_MSG);

  const size_t vector_byte_size = vector_input_data.size() * sizeof(Type);

  if (std::optional<ByteVectorWithCounter> optional_output_buffer_of_bytes{
          vector_byte_size};
      write_vector_deep_copy<Version, ByteVectorWithCounter,
                             picklejar::write_object_to_buffer>(
          vector_input_data, optional_output_buffer_of_bytes.value(),
          element_size_getter_lambda, write_element_lambda))
    return optional_output_buffer_of_bytes;
  return {};
}

template <size_t Version = 0, class BufferOrStreamObject,
          bool WriteSizeFunction(const size_t &, BufferOrStreamObject &) =
              picklejar::write_object_to_stream<size_t>,
          class Type, class WriteElementLambda>
auto deep_copy_object_to_buffer(const Type &object, const size_t object_size,
                                WriteElementLambda &&write_element_lambda)
    -> std::optional<ByteVectorWithCounter> {
  PICKLEJAR_CONCEPT(
      (PickleJarWriteLambdaRequirements<WriteElementLambda,
                                        BufferOrStreamObject, Type>),
      WRITELAMBDAREQUIREMENTS_MSG);

  constexpr size_t vector_byte_size = sizeof(Type);

  if (std::optional<ByteVectorWithCounter> optional_output_buffer_of_bytes{
          vector_byte_size};
      write_object_deep_copy<Version, ByteVectorWithCounter,
                             picklejar::write_object_to_buffer>(
          object, object_size, optional_output_buffer_of_bytes.value(),
          write_element_lambda))
    return optional_output_buffer_of_bytes;
  return {};
}
// START object_buffer_v1_copy uses object_buffer_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ByteContainerOrViewType>
[[nodiscard]] constexpr auto read_object_from_buffer(
    ByteContainerOrViewType &buffer_with_input_bytes) -> std::optional<Type> {
  PICKLEJAR_CONCEPT(
      PickleJarValidByteContainerOrViewType<ByteContainerOrViewType>,
      VALIDBYTECONTAINERORVIEWTYPE_MSG);
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  return buffer_with_input_bytes.template read<Type, ManagedAlignedCopy>();
}

// END object_buffer_v1_copy uses object_buffer_v1

template <size_t Version = 0, class Container,
          typename Type = typename Container::value_type,
          class VectorInsertElementLambda, class ByteContainerOrViewType>
auto deep_read_vector_from_buffer(
    Container &result, ByteContainerOrViewType &vector_byte_buffer,
    VectorInsertElementLambda &&vector_insert_element_lambda)
    -> picklejar::optional<Container> {
  PICKLEJAR_CONCEPT(
      PickleJarValidByteContainerOrViewType<ByteContainerOrViewType>,
      VALIDBYTECONTAINERORVIEWTYPE_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarVectorInsertElementLambdaRequirements<VectorInsertElementLambda,
                                                      Container>),
      VECTORINSERTELEMENTLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(ContainerDeepCopyReadRequirements<Container>,
                    CONTAINERDEEPCOPYREADREQUIREMENTS_MSG);
  return read_vector_deep_copy<Version, ByteVectorWithCounter,
                               picklejar::read_object_from_buffer<size_t>,
                               picklejar::basic_buffer_read>(
      result, vector_byte_buffer, vector_insert_element_lambda);
}

template <size_t Version = 0, class BufferOrStreamObject,
          std::optional<size_t> ReadSizeFunction(BufferOrStreamObject &) =
              picklejar::read_object_from_stream<size_t>,
          bool ReadBufferOrStreamFunction(BufferOrStreamObject &, char *,
                                          const size_t) =
              picklejar::basic_stream_read,
          class ByteBufferLambda>
auto deep_read_object_to_buffer(BufferOrStreamObject &buffer_or_stream_object,
                                ByteBufferLambda &&byte_buffer_lambda) -> bool {
  PICKLEJAR_CONCEPT((PickleJarByteBufferLambdaRequirements<ByteBufferLambda>),
                    BYTEBUFFERLAMBDAREQUIREMENTS_MSG);
  return read_object_deep_copy<Version, ByteVectorWithCounter,
                               picklejar::read_object_from_buffer<size_t>,
                               picklejar::basic_buffer_read>(
      buffer_or_stream_object, byte_buffer_lambda);
}

// END DEEP COPY FUNCTIONS

// functions we needed after for convenience
template <class PointerType>
[[nodiscard]] auto basic_stream_write(std::ofstream &ofs_output_file,
                                      PointerType *destination_to_copy_to,
                                      const size_t size_to_read) -> bool {
  ofs_output_file.write(reinterpret_cast<const char *>(destination_to_copy_to),
                        std::streamsize(size_to_read));  // NOLINT
  return ofs_output_file.good();
}

template <class PointerType>
[[nodiscard]] auto basic_buffer_write(
    ByteVectorWithCounter &byte_vector_with_counter,
    PointerType *destination_to_copy_to, const size_t size_to_read) -> bool {
  byte_vector_with_counter.write(destination_to_copy_to, size_to_read);
  return true;
}

inline auto read_version_from_stream(std::ifstream &ifstream_input_file)
    -> std::optional<size_t> {
  return picklejar::read_object_from_stream<size_t>(ifstream_input_file);
}
inline auto read_version_from_file(const std::string file_name)
    -> std::optional<size_t> {
  std::ifstream ifstream_input_file(file_name);
  return picklejar::read_object_from_stream<size_t>(ifstream_input_file);
}
template <class ByteContainerOrViewType>
auto read_version_from_buffer(ByteContainerOrViewType &byte_vector_with_counter)
    -> std::optional<size_t> {
  PICKLEJAR_CONCEPT(
      PickleJarValidByteContainerOrViewType<ByteContainerOrViewType>,
      VALIDBYTECONTAINERORVIEWTYPE_MSG);
  return picklejar::read_object_from_buffer<size_t>(byte_vector_with_counter);
}

template <size_t Version = 0>
constexpr auto versioned_size() -> size_t {
  if constexpr (Version > 0) {
    // version + size as header
    return sizeof(size_t) * 2;
  } else {
    // just the size as header
    return sizeof(size_t);
  }
}

template <typename C>
concept IsIterable = requires(C c) {
  { c.cbegin() } -> std::same_as<typename C::const_iterator>;
  { c.cend() } -> std::same_as<typename C::const_iterator>;
  { c.size() } -> std::same_as<size_t>;
};
template <typename C>
concept NotIterable = requires(C c) {
  requires !IsIterable<C>;
};

template <typename C>
concept IsMapType = requires(C c) {
  requires !std::same_as<typename C::key_type, void>;
  requires !std::same_as<typename C::mapped_type, void>;
};

template <size_t Version = 0, NotIterable Object>
constexpr auto sizeof_versioned(Object object) -> size_t {
  return picklejar::versioned_size<Version>() + sizeof(object);
}
template <size_t Version = 0, IsIterable Container>
constexpr auto sizeof_versioned(Container container) -> size_t {
  if constexpr (IsMapType<Container>) {
    if constexpr (std::same_as<std::string, typename Container::key_type>) {
      // if you get a trivially_copiable warning here is because when I wrote
      // this I assumed the value_type of the map would be trivially copiable,
      // if that is not the case you will have to copy this function and make it
      // return the size of what you are writting
      PICKLEJAR_CONCEPT(CanBeCopiedEasily<typename Container::value_type>,
                        TRIVIALLYCOPIABLE_MSG);
      return versioned_size<Version>() +
             std::transform_reduce(
                 std::cbegin(container), std::cend(container), size_t{0},
                 std::plus<>(), [](auto &map_elem) {
                   return versioned_size<0>() + sizeof(size_t) +
                          map_elem.first.size() + sizeof(map_elem.second);
                 });
    } else {
      PICKLEJAR_CONCEPT(CanBeCopiedEasily<typename Container::key_type>,
                        TRIVIALLYCOPIABLE_MSG);
      PICKLEJAR_CONCEPT(CanBeCopiedEasily<typename Container::value_type>,
                        TRIVIALLYCOPIABLE_MSG);
      return versioned_size<Version>() +
             std::transform_reduce(
                 std::cbegin(container), std::cend(container), size_t{0},
                 std::plus<>(), [](auto &map_elem) {
                   return versioned_size<0>() + sizeof(map_elem.first) +
                          sizeof(map_elem.second);
                 });
    }
  } else {
    PICKLEJAR_CONCEPT(CanBeCopiedEasily<typename Container::value_type>,
                      TRIVIALLYCOPIABLE_MSG);
    return picklejar::versioned_size<Version>() +
           (container.size() *
            (versioned_size<0>() + sizeof(typename Container::value_type)));
  }
}

template <class BufferOrStreamObject>
auto string_write_generic(const std::string string_to_write,
                          BufferOrStreamObject &buffer_or_stream_object)
    -> bool {
  if constexpr (std::same_as<BufferOrStreamObject, std::ofstream>) {
    // write the actual size of our string into the file
    if (!write_object_to_stream(string_to_write.size(),
                                buffer_or_stream_object))
      return false;

    // next we write the string data into the file
    if (!basic_stream_write(buffer_or_stream_object, string_to_write.data(),
                            string_to_write.size()))
      return false;
  } else {
    // write the actual size of our string into the file
    if (!write_object_to_buffer(string_to_write.size(),
                                buffer_or_stream_object))
      return false;

    // next we write the string data into the file
    if (!basic_buffer_write(buffer_or_stream_object, string_to_write.data(),
                            string_to_write.size()))
      return false;
  }
  return true;
}

inline auto write_string_to_stream(const std::string string_to_write,
                                   std::ofstream &_ofs_output_file) -> bool {
  return string_write_generic(string_to_write, _ofs_output_file);
}
inline auto write_string_to_file(const std::string string_to_write,
                                 const std::string file_name) -> bool {
  std::ofstream _ofs_output_file(file_name);
  return string_write_generic(string_to_write, _ofs_output_file);
}

inline auto write_string_to_buffer(
    const std::string string_to_write,
    ByteVectorWithCounter &byte_vector_with_counter) -> bool {
  return string_write_generic(string_to_write, byte_vector_with_counter);
}

template <NotIterable Object>
constexpr auto sizeof_unversioned(Object object) -> size_t {
  PICKLEJAR_CONCEPT(CanBeCopiedEasily<Object>, TRIVIALLYCOPIABLE_MSG);
  return sizeof(Object);
}

template <IsIterable Container>
constexpr auto sizeof_unversioned(Container container) -> size_t {
  static_assert(
      !IsMapType<Container>,
      "PICKLEJAR_HELP: You need to use deep_copy functions for map types");
  PICKLEJAR_CONCEPT(CanBeCopiedEasily<typename Container::value_type>,
                    TRIVIALLYCOPIABLE_MSG);
  return container.size() * sizeof(typename Container::value_type);
}

inline auto sizeof_unversioned(std::string string_to_get_size_of) -> size_t {
  return /*count bytes to store result of .size() */
      sizeof(size_t) +
      /*followed by how many characters our string has*/
      string_to_get_size_of.size();
}

}  // namespace picklejar
#endif
