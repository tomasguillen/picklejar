// This file was written to find out if returning an optional with an object or
// a vector inside is efficient at all(Aka: does RVO apply to optionals?)

#include <string>
#include <tuple>
#include <utility>
#include <vector>

template <typename T>
struct TestVector : public std::vector<T> {
  using std::vector<T>::push_back;
  using std::vector<T>::emplace_back;
  using std::vector<T>::reserve;
  using std::vector<T>::size;

  template <typename... Param>
  explicit TestVector() : std::vector<T>() {
    std::puts("TestVector()");
  }

  auto operator=(const TestVector &rhs) noexcept -> const TestVector & = delete;
  auto operator=(TestVector &&rhs) noexcept -> TestVector & = delete;
  TestVector(const TestVector &rhs) noexcept : std::vector<T>(rhs) {
    std::puts("copy()");
  }
  TestVector(TestVector &&rhs) noexcept : std::vector<T>(rhs) {
    std::puts("move()");
  }
};

#include "../picklejar.hpp"
static void exampleSolution3() {
  TestVector<std::string> string_vec;
  string_vec.push_back("string1");
  string_vec.push_back("string2");
  string_vec.push_back("string3");
  string_vec.push_back("string4");
  string_vec.push_back("string5");
  string_vec.push_back("string6");
  string_vec.push_back("string7");
  string_vec.push_back("string8");
  string_vec.push_back("string9");
  string_vec.push_back("string10");
  string_vec.push_back("string11");

  if (picklejar::write_vector_to_file(string_vec, "example1.data"))
    std::puts("WRITESUCCESS");

  if (auto optional_read_vector = picklejar::read_vector_from_file<std::string>(
          "example1.data",
          [](auto &blank_instance, auto &valid_bytes_from_new_blank_instance,
             auto &bytes_from_file) {
            picklejar::util::preserve_blank_instance_member(
                0, sizeof(std::string), valid_bytes_from_new_blank_instance,
                bytes_from_file);
            picklejar::util::copy_new_bytes_to_instance(
                valid_bytes_from_new_blank_instance, blank_instance,
                sizeof(std::string));
          },
          [count = 0]() mutable {
            // we return a tuple with the parameters used to construct our
            // non-trivially copiable type
            return make_tuple("string" + std::to_string(++count));
          });
      optional_read_vector.has_value()) {
    std::puts(
        ("READSUCCESS: fifth_element=" + optional_read_vector.value().at(4))
            .c_str());
  }
  TestVector hello = string_vec;
}

static auto exampleOptionalReturn(std::vector<int> &ref_vector)
    -> std::optional<std::vector<int>> {
  return std::make_optional(ref_vector);
}

auto main() -> int {
  // exampleSolution3();
  std::vector<int> ref_vector{1, 2, 3, 4};
  auto hello{exampleOptionalReturn(ref_vector)};
  std::puts(std::to_string(hello.value().at(2)).c_str());
  return EXIT_SUCCESS;
}
