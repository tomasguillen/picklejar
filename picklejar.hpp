#ifndef PICKLEJAR_H  // This is the include guard macro
#define PICKLEJAR_H 1
#include <array>
#include <cstring>
#include <fstream>
#include <limits>
#include <optional>
#include <span>
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
  "lambda to fix non trivially copiable members. SEE NON TRIVIAL EXAMPLES "    \
  "section in the readme file"

#define DEFAULTCONSTRUCTIBLE_MSG                                              \
  "PICKLEJAR_HELP: The object passed to picklejar is not default "            \
  "constructible. "                                                           \
  "You will need to pass a fourth parameter to picklejar with a lambda that " \
  "returns a tuple with the parameters to construct the object. SEE NON "     \
  "TRIVIAL EXAMPLES section in the readme file"

// LAMBDA CONCEPTS

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

// DEEP COPY CONCEPTS
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

template <typename ElementSizeGetterLambda, typename Type>
concept PickleJarElementSizeGetterRequirements =
    requires(ElementSizeGetterLambda function, Type templated_object) {
  { function(templated_object) } -> std::same_as<size_t>;
};

struct ByteVectorWithCounter;  // forward declaration just for bytebufferlambda
                               // concept
template <typename ByteBufferLambda>
concept PickleJarByteBufferLambdaRequirements = requires(
    ByteBufferLambda function, ByteVectorWithCounter byte_vector_with_counter) {
  { function(byte_vector_with_counter) } -> std::same_as<bool>;
};

template <typename VectorInsertElementLambda, typename Container>
concept PickleJarVectorInsertElementLambdaRequirements =
    requires(VectorInsertElementLambda function, Container container,
             ByteVectorWithCounter byte_vector_with_counter) {
  { function(container, byte_vector_with_counter) } -> std::same_as<bool>;
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

#define PICKLEJAR_CONCEPT(conditional, message) \
  static_assert(conditional, message)
#else
#define PICKLEJAR_CONCEPT(conditional, message)
#endif
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
    -> std::optional<Container> {
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
  if (ifstream_close_and_check_is_invalid(ifstream_input_file)) {
    return {};
  }
  return result;
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
using BufferContainer = std::span<char>;

// START object_buffer_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ManipulateBytesLambda>
auto operation_specific_read_object_from_buffer(
    ManagedAlignedCopy &copy, BufferContainer buffer_with_input_bytes,
    size_t &bytes_read_so_far,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> ManagedAlignedCopy & {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<BufferContainer>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);

  std::array<char, sizeof(Type)> valid_bytes_blank_instance_copy{};
  std::memcpy(valid_bytes_blank_instance_copy.data(),
              copy.get_pointer_to_copy(), sizeof(Type));
  std::array<char, sizeof(Type)>
      bytes_from_file{};  // TODO(tom): this may be unnecessary we could just
                          // pass buffer_with_input_bytes with offset and size?
  std::memcpy(bytes_from_file.data(),
              buffer_with_input_bytes.data() + bytes_read_so_far, sizeof(Type));
  bytes_read_so_far += sizeof(Type);
  manipulate_bytes_from_file_before_writing_to_instance_lambda(
      *copy.get_pointer_to_copy(), valid_bytes_blank_instance_copy,
      bytes_from_file);
  return copy;
}
// END object_buffer_v2
// START object_buffer_v2_copy uses object_buffer_v2
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class ManipulateBytesLambda>
auto read_object_from_buffer(
    BufferContainer buffer_with_input_bytes, size_t &bytes_read_so_far,
    ManipulateBytesLambda &&
        manipulate_bytes_from_file_before_writing_to_instance_lambda) -> Type {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<BufferContainer>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
  ManagedAlignedCopy copy{};
  return *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
               copy, buffer_with_input_bytes, bytes_read_so_far,
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
          class ConstructorGeneratorLambda, class ManipulateBytesLambda>
[[nodiscard]] auto read_object_from_buffer(
    BufferContainer buffer_with_input_bytes, size_t &bytes_read_so_far,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda,
    ConstructorGeneratorLambda &&constructor_generator_lambda) -> Type {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT((PickleJarConstructorGeneratorLambdaRequirements<
                        ConstructorGeneratorLambda, Type>),
                    CONSTRUCTORGENERATORLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<BufferContainer>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
  ManagedAlignedCopy copy{constructor_generator_lambda()};
  return *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
               copy, buffer_with_input_bytes, bytes_read_so_far,
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
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class BufferContainer>
[[nodiscard]] auto operation_specific_read_object_from_buffer(
    ManagedAlignedCopy &copy, BufferContainer buffer_with_input_bytes,
    size_t &bytes_read_so_far) -> ManagedAlignedCopy & {
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<BufferContainer>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
  std::memcpy(copy.get_pointer_to_copy(),
              buffer_with_input_bytes.data() + bytes_read_so_far, sizeof(Type));
  bytes_read_so_far += sizeof(Type);
  return copy;
}
// END object_buffer_v1
// START object_buffer_v1_copy uses object_buffer_v1
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class BufferContainer>
[[nodiscard]] auto read_object_from_buffer(
    BufferContainer buffer_with_input_bytes, size_t &bytes_read_so_far)
    -> Type {
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<BufferContainer>,
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
          class Container>
[[nodiscard]] constexpr auto read_vector_from_buffer(
    Container &vector_input_data, BufferContainer buffer_with_input_bytes)
    -> std::optional<Container> {
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  PICKLEJAR_CONCEPT((ContainerHasPushBack<Container, Type>),
                    CONTAINERWITHHASPUSHBACK_MSG);
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<BufferContainer>,
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
    if (bytes_read_so_far + sizeof(Type) > file_size) break;
    ManagedAlignedCopy copy{};
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
          class Container, class ConstructorGeneratorLambda,
          class ManipulateBytesLambda>
[[nodiscard]] constexpr auto read_vector_from_buffer(
    Container &vector_input_data, BufferContainer buffer_with_input_bytes,
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
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<BufferContainer>,
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
    // std::puts(
    //    ("bytes_read_so_far: " + std::to_string(bytes_read_so_far)).c_str());
    if (bytes_read_so_far + sizeof(Type) > file_size) break;
    // START OPERATION VERSION
    ManagedAlignedCopy copy{constructor_generator_lambda()};

    // END OPERATION VERSION
    vector_input_data.push_back(std::move(
        *(operation_specific_read_object_from_buffer<Type, ManagedAlignedCopy>(
              copy, buffer_with_input_bytes, bytes_read_so_far,
              manipulate_bytes_from_file_before_writing_to_instance_lambda))
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
          class Container, class ConstructorGeneratorLambda,
          class ManipulateBytesLambda>
[[nodiscard]] auto read_vector_from_stream(
    Container &vector_input_data, std::ifstream &ifstream_input_file,
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
    return std::make_optional<Container>(std::move(vector_input_data));
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
    -> std::optional<Container> {
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
template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>,
          class Container, class ManipulateBytesLambda>
[[nodiscard]] constexpr auto read_vector_from_buffer(
    Container &vector_input_data, BufferContainer buffer_with_input_bytes,
    ManipulateBytesLambda
        &&manipulate_bytes_from_file_before_writing_to_instance_lambda)
    -> std::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarManipulateBytesLambdaRequirements<ManipulateBytesLambda, Type>),
      MANIPULATEBYTESLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(DefaultConstructible<Type>, DEFAULTCONSTRUCTIBLE_MSG);
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<BufferContainer>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
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
  if (ifstream_close_and_check_is_invalid(ifstream_input_file)) {
    return {};
  }
  return result;
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
  if (ifstream_close_and_check_is_invalid(ifstream_input_file)) {
    return {};
  }
  return result;
}
// END file_v3 uses stream_v3
// END READ_API

// DEEP COPY FUNCTIONS
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
    return write_element_lambda(buffer_or_stream_object, object,
                                object_size);  // NOLINT
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
      // for each element we write the size of the object first
      if (!write_object_deep_copy<0, BufferOrStreamObject, WriteSizeFunction>(
              object, element_size_getter_lambda(object),
              buffer_or_stream_object, write_element_lambda))
        return false;
    }
    return true;
  }
  return false;
}

struct ByteVectorWithCounter {
  std::vector<char> byte_data{};
  std::optional<size_t> byte_counter{0};
  explicit ByteVectorWithCounter(size_t number_of_bytes)
      : byte_data(number_of_bytes) {}
  explicit ByteVectorWithCounter(std::vector<char> &_byte_data)
      : byte_data(_byte_data) {}

  [[nodiscard]] auto size() const { return byte_data.size(); }
  [[nodiscard]] auto size_remaining() const {
    return byte_counter ? size() - byte_counter.value() : 0;
  }

  auto get_remaining_bytes_as_vector() -> std::vector<char> {
    return {std::begin(byte_data) + int(byte_counter.value()),
            std::end(byte_data)};
  }

  auto get_remaining_bytes_as_span() -> std::span<char> {
    return {byte_data.data() + byte_counter.value(), size_remaining()};
  }

  void set_counter(size_t new_counter) { byte_counter = new_counter; }

  [[nodiscard]] auto would_it_be_full_if_so_invalidate(size_t size_to_advance)
      -> bool {
    if (size_to_advance > size_remaining()) {
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

  template <class Type>
  auto read() -> std::optional<Type> {
    if (would_it_be_full_if_so_invalidate(sizeof(Type))) return {};
    return read_object_from_buffer<Type>(byte_data, byte_counter.value());
  }
  template <class PointerType>
  auto read(PointerType *destination_to_copy_to, const size_t size_to_read)
      -> bool {
    if (would_it_be_full_if_so_invalidate(size_to_read)) return false;
    std::memcpy(reinterpret_cast<char *>(destination_to_copy_to),
                byte_data.data() + byte_counter.value(),  // NOLINT
                size_to_read);
    byte_counter.value() += size_to_read;
    return true;
  }
  auto begin() { return std::begin(byte_data); }
  auto end() { return std::end(byte_data); }
  friend auto begin(ByteVectorWithCounter &byte_vector_with_counter) {
    return byte_vector_with_counter.begin();
  }
  friend auto end(ByteVectorWithCounter &byte_vector_with_counter) {
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
  [[nodiscard]] auto invalid() const -> bool {
    return !byte_counter.has_value();
  }
};

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
      return false;
    }
  }

  if (auto optional_string_size = ReadSizeFunction(buffer_or_stream_object)) {
    // if we got the size of our object in optional_string_size.value()
    // we create a vector of char and we read the stream into it
    ByteVectorWithCounter byte_buffer(optional_string_size.value());
    if (ReadBufferOrStreamFunction(buffer_or_stream_object,
                                   byte_buffer.byte_data.data(),
                                   optional_string_size.value())) {
      // if read is sucessful we create the object using it's byte_buffer bytes
      return byte_buffer_lambda(byte_buffer);
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
    -> std::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarVectorInsertElementLambdaRequirements<VectorInsertElementLambda,
                                                      Container>),
      VECTORINSERTELEMENTLAMBDAREQUIREMENTS_MSG);
  size_t result_initial_size{result.size()};
  if constexpr (Version > 0) {
    if (auto optional_version = ReadSizeFunction(buffer_or_stream_object);
        !optional_version or optional_version.value() != Version) {
      return {};
    }
  }
  if (auto optional_size = ReadSizeFunction(buffer_or_stream_object)) {
    result.reserve(optional_size.value());
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
    return std::make_optional(std::move(result));
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
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<Container>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
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
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<Container>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
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
    -> std::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarVectorInsertElementLambdaRequirements<VectorInsertElementLambda,
                                                      Container>),
      VECTORINSERTELEMENTLAMBDAREQUIREMENTS_MSG);

  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<Container>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
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
    -> std::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarVectorInsertElementLambdaRequirements<VectorInsertElementLambda,
                                                      Container>),
      VECTORINSERTELEMENTLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<Container>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
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
auto deep_read_object_to_file(const std::string file_name,
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
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<Container>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarWriteLambdaRequirements<WriteElementLambda,
                                        ByteVectorWithCounter, Type>),
      WRITELAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(
      (PickleJarElementSizeGetterRequirements<ElementSizeGetterLambda, Type>),
      SIZEGETTERLAMBDAREQUIREMENTS_MSG);

  const size_t vector_byte_size = vector_input_data.size() * sizeof(Type);
  ByteVectorWithCounter output_buffer_of_bytes(vector_byte_size);
  if (write_vector_deep_copy<Version, ByteVectorWithCounter,
                             picklejar::write_object_to_buffer>(
          vector_input_data, output_buffer_of_bytes, element_size_getter_lambda,
          write_element_lambda))
    return output_buffer_of_bytes;
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
  ByteVectorWithCounter output_buffer_of_bytes(vector_byte_size);
  if (write_object_deep_copy<Version, ByteVectorWithCounter,
                             picklejar::write_object_to_buffer>(
          object, object_size, output_buffer_of_bytes, write_element_lambda))
    return output_buffer_of_bytes;
  return {};
}

template <class Type,
          class ManagedAlignedCopy = ManagedAlignedCopyDefault<Type>>
[[nodiscard]] auto read_object_from_buffer(
    ByteVectorWithCounter &buffer_with_input_bytes) -> std::optional<Type> {
  PICKLEJAR_CONCEPT(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
  return buffer_with_input_bytes.read<Type>();
}

template <size_t Version = 0, class Container,
          typename Type = typename Container::value_type,
          class VectorInsertElementLambda>
auto deep_read_vector_from_buffer(
    Container &result, ByteVectorWithCounter &vector_byte_buffer,
    VectorInsertElementLambda &&vector_insert_element_lambda)
    -> std::optional<Container> {
  PICKLEJAR_CONCEPT(
      (PickleJarVectorInsertElementLambdaRequirements<VectorInsertElementLambda,
                                                      Container>),
      VECTORINSERTELEMENTLAMBDAREQUIREMENTS_MSG);
  PICKLEJAR_CONCEPT(ContainerHasDataAndSize<Container>,
                    CONTAINERWITHHASDATAANDSIZE_MSG);
  ByteVectorWithCounter byte_vector_with_counter(vector_byte_buffer);
  return read_vector_deep_copy<Version, ByteVectorWithCounter,
                               picklejar::read_object_from_buffer<size_t>,
                               picklejar::basic_buffer_read>(
      result, byte_vector_with_counter, vector_insert_element_lambda);
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

inline auto read_version_from_stream(std::ifstream &ifstream_input_file)
    -> std::optional<size_t> {
  return picklejar::read_object_from_stream<size_t>(ifstream_input_file);
}
inline auto read_version_from_file(const std::string file_name)
    -> std::optional<size_t> {
  std::ifstream ifstream_input_file(file_name);
  return picklejar::read_object_from_stream<size_t>(ifstream_input_file);
}
inline auto read_version_from_buffer(
    ByteVectorWithCounter &byte_vector_with_counter) -> std::optional<size_t> {
  return picklejar::read_object_from_buffer<size_t>(byte_vector_with_counter);
}

}  // namespace picklejar
#endif
