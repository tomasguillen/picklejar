#include <cassert>

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

#include "../tests/hexer/hexer.hpp"
#include "../picklejar.hpp"

struct SimpleStructure {
  int byte2_note_range_start, byte2_note_range_end, byte3_item_current_idx,
      byte3_vel;
  enum class BYTE3 { any, lessthan, morethan };
  auto operator==(const SimpleStructure &comp) const -> bool {
    return byte2_note_range_start == comp.byte2_note_range_start &&
           byte2_note_range_end == comp.byte2_note_range_end &&
           byte3_item_current_idx == comp.byte3_item_current_idx &&
           byte3_vel == comp.byte3_vel;
  }
};

struct ComplexStructure {
  SimpleStructure note_range_selector;
  int status_byte_item_current_idx{}, transpose_n_notes{}, midi_channel = 1;
  std::string id{};
  bool marked_for_deletion{false};
  bool marked_for_move{false};
  bool marked_move_direction_up{false};
  bool marked_move_direction_down{true};

  enum class STATUSBYTE { NotesRange, Control };
  auto operator==(const ComplexStructure &comp) const -> bool {
    return note_range_selector == comp.note_range_selector &&
           status_byte_item_current_idx == comp.status_byte_item_current_idx &&
           transpose_n_notes == comp.transpose_n_notes &&
           midi_channel == comp.midi_channel;
  }
  ComplexStructure() noexcept
      : note_range_selector{0, 127, 0, 63},
        status_byte_item_current_idx{0},
        transpose_n_notes{0} {
    std::puts("ComplexStructure()");
  }
  ComplexStructure(const ComplexStructure &rhs) noexcept {
    std::puts(("copy(" + rhs.id + ")").c_str());
    id = rhs.id;
    note_range_selector = rhs.note_range_selector;
    status_byte_item_current_idx = rhs.status_byte_item_current_idx;
    transpose_n_notes = rhs.transpose_n_notes;
    midi_channel = rhs.midi_channel;
  }
  ComplexStructure(ComplexStructure &&rhs) noexcept {
    std::puts(("move(" + rhs.id + ")").c_str());
    id = std::exchange(rhs.id, {});
    note_range_selector = std::exchange(rhs.note_range_selector, {});
    status_byte_item_current_idx =
        std::exchange(rhs.status_byte_item_current_idx, {});
    transpose_n_notes = std::exchange(rhs.transpose_n_notes, {});
    midi_channel = std::exchange(rhs.midi_channel, {});
  }

  explicit ComplexStructure(auto _id) noexcept
      : id{_id},
        note_range_selector{0, 127, 0, 63},
        status_byte_item_current_idx{8738},
        transpose_n_notes{0} {
    std::puts((std::string("ComplexStructure(") + _id + ")").c_str());
  }
  explicit ComplexStructure(auto _id, auto _id2) noexcept
      : id{_id2},
        note_range_selector{0, 127, 0, 63},
        status_byte_item_current_idx{8738},
        transpose_n_notes{0} {
    std::puts((std::string("ComplexStructure(") + _id + ")").c_str());
  }

  ~ComplexStructure() {
    std::puts((("~(") +
               // std::to_string(note_range_selector.byte2_note_range_start)
               id + ")")
                  .c_str());
    ;
  }
  void draw();
};
inline void print_vec(std::vector<ComplexStructure> &vector_data) {
  std::cout << "Reading contents of vector: \n";
  for (auto &val : vector_data) {
    std::cout << val.id << "= " << val.note_range_selector.byte2_note_range_end
              << ":" << val.midi_channel << ", ";
  }
  std::cout << "\nEND\n";
}

static void example1() {
  std::vector<int> int_vec{0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

  if (picklejar::write_vector_to_file(int_vec, "example1.data"))
    std::puts("WRITESUCCESS");

  if (auto optional_read_vector =
          picklejar::read_vector_from_file<int>("example1.data");
      optional_read_vector) {
    std::puts(("READSUCCESS: fourth element is " +
               std::to_string(optional_read_vector.value().at(4)))
                  .c_str());
  }
}
static void example2a() {
  /*std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",  "16",
                                      "32", "64", "128", "256", "512"};

  if (picklejar::write_vector_to_file(string_vec, "example1.data"))
    std::puts("WRITESUCCESS");

  if (auto optional_read_vector = picklejar::read_vector_from_file<std::string>(
          "example1.data");  // This results in compiler error because
                             // std::string is non-trivially-copiable
      optional_read_vector.has_value()) {
    std::puts(
        ("READSUCCESS: fourth element is " + optional_read_vector.value().at(4))
            .c_str());
  }*/
}
static void example2b() {
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",  "16",
                                      "32", "64", "128", "256", "512"};

  if (picklejar::write_vector_to_file(string_vec, "example1.data"))
    std::puts("WRITESUCCESS");

  if (auto optional_read_vector = picklejar::read_vector_from_file<std::string>(
          "example1.data",
          [count = 1](auto &blank_instance,
                      auto &valid_bytes_from_new_blank_instance,
                      auto &bytes_from_file) mutable {
            picklejar::util::preserve_blank_instance_member(
                0, sizeof(std::string), valid_bytes_from_new_blank_instance,
                bytes_from_file);
            picklejar::util::copy_new_bytes_to_instance(
                valid_bytes_from_new_blank_instance, blank_instance,
                sizeof(std::string));
            blank_instance = "string" + std::to_string(++count);
          });
      optional_read_vector.has_value()) {
    std::puts(("READSUCCESS: last_element=" +
               optional_read_vector.value().at(
                   optional_read_vector.value().size() - 1))
                  .c_str());
    hexer::print_vec(optional_read_vector.value());
  }
}

static void exampleSolution1a() {
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",  "16",
                                      "32", "64", "128", "256", "512"};

  std::ofstream ofs_output_file("example1.data");
  if (picklejar::write_object_to_stream(string_vec.size(), ofs_output_file)) {
    std::puts("WRITE_VECTOR_SIZE_SUCCESS");
    for (auto object : string_vec) {
      // for each element we write the size of the string first
      if (picklejar::write_object_to_stream(object.size(), ofs_output_file)) {
        std::puts("WRITE_ELEMENT_SIZE_SUCCESS");
        // then we read the characters of the string
        ofs_output_file.write(object.data(),
                              std::streamsize(object.size()));  // NOLINT
        if (ofs_output_file.good()) {
          std::puts("WRITE_ELEMENT_SUCCESS");
        } else {
          std::puts("WRITE_ELEMENT_ERROR");
          break;
        }
      } else {
        break;
      }
    }
  }
  ofs_output_file.close();

  std::ifstream ifs_input_file("example1.data");
  std::vector<std::string> result;
  if (auto optional_size =
          picklejar::read_object_from_stream<size_t>(ifs_input_file)) {
    std::puts("READ_VECTOR_SIZE_SUCCESS");
    result.reserve(optional_size.value());
    for (size_t i{0}; i < optional_size.value(); ++i) {
      if (auto optional_string_size =
              picklejar::read_object_from_stream<size_t>(ifs_input_file)) {
        std::puts("READ_ELEMENT_SIZE_SUCCESS");
        // if we got the size of our string in optional_string_size.value()
        // we create a vector of char and we read the stream into it
        std::vector<char> char_buffer(optional_string_size.value());
        ifs_input_file.read(char_buffer.data(),
                            std::streamsize(optional_string_size.value()));
        if (ifs_input_file.good()) {
          std::puts("READ_ELEMENT_SUCCESS");
          // if read is sucessful we create the string using the char_buffer
          // iterators
          result.emplace_back(std::begin(char_buffer), std::end(char_buffer));
        } else {
          std::puts("READ_ELEMENT_ERROR");
          break;
        }
      } else {
        std::puts("READ_ELEMENT_ERROR");
        break;
      }
    }
  }
  ifs_input_file.close();

  std::puts(("fifth element=" + result.at(4)).c_str());

  hexer::print_vec(result);
}

static auto store_object(auto &object, size_t object_size,
                         std::ofstream &ofs_output_file,
                         auto &&element_write_lambda) -> bool {
  if (picklejar::write_object_to_stream(object_size, ofs_output_file)) {
    element_write_lambda(ofs_output_file, object, object_size);  // NOLINT
    return ofs_output_file.good();
  }
  return false;
}

template <class Type>
static auto store_vector(std::vector<Type> &string_vec,
                         std::ofstream &ofs_output_file,
                         auto &&element_size_getter,
                         auto &&element_write_lambda) -> bool {
  if (!string_vec.empty() and
      picklejar::write_object_to_stream(string_vec.size(), ofs_output_file)) {
    for (auto &object : string_vec) {
      // for each element we write the size of the object first
      if (!store_object(object, element_size_getter(object), ofs_output_file,
                        element_write_lambda))
        return false;
    }
    return true;
  }
  return false;
}

static auto read_object(std::ifstream &ifs_input_file,
                        auto &&byte_buffer_function) {
  if (auto optional_string_size =
          picklejar::read_object_from_stream<size_t>(ifs_input_file)) {
    // if we got the size of our object in optional_string_size.value()
    // we create a vector of char and we read the stream into it
    std::vector<char> char_buffer(optional_string_size.value());
    ifs_input_file.read(char_buffer.data(),
                        std::streamsize(optional_string_size.value()));
    if (ifs_input_file.good()) {
      // if read is sucessful we create the object using it's char_buffer bytes
      byte_buffer_function(char_buffer);
      return true;
    }
  }
  return false;
}

template <class Type>
static auto read_vector(std::vector<Type> &result,
                        std::ifstream &ifs_input_file,
                        auto &&vector_insert_element_function)
    -> std::optional<std::vector<Type>> {
  size_t result_initial_size{result.size()};
  if (auto optional_size =
          picklejar::read_object_from_stream<size_t>(ifs_input_file)) {
    result.reserve(optional_size.value());
    for (size_t i{0}; i < optional_size.value(); ++i) {
      read_object(ifs_input_file, [&](auto &char_buffer) {
        vector_insert_element_function(result, char_buffer);
      });
    }
  }
  if (result.size() > result_initial_size) {
    return std::make_optional(result);
  }
  return {};
}

static void exampleSolution1b() {
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",   "16",
                                      "32", "64", "128", "256", "512", "1000"};

  std::ofstream ofs_output_file("example1.data");
  if (store_vector(
          string_vec, ofs_output_file,
          [](auto &string) { return string.size(); },
          [](auto &_ofs_output_file, auto &object, size_t element_size) {
            _ofs_output_file.write(object.data(),
                                   std::streamsize(element_size));
          })) {
    std::puts("WRITE_SUCCESS");
  }
  ofs_output_file.close();

  std::ifstream ifs_input_file("example1.data");
  std::vector<std::string> result;
  if (auto optional_result{read_vector(
          result, ifs_input_file, [](auto &_result, auto &char_buffer) {
            _result.emplace_back(std::begin(char_buffer),
                                 std::end(char_buffer));
          })}) {
    std::puts(("fifth element=" + optional_result.value().at(4)).c_str());
  }
  ifs_input_file.close();

  hexer::print_vec(result);
}

static void exampleSolution1c() {
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",   "16",
                                      "32", "64", "128", "256", "512", "1024"};

  std::ofstream ofs_output_file("example1.data");
  if (picklejar::write_vector_deep_copy(
          string_vec, ofs_output_file,
          [](auto &string) { return string.size(); },
          [](auto &_ofs_output_file, auto &object, size_t element_size) {
            _ofs_output_file.write(object.data(),
                                   std::streamsize(element_size));
            return _ofs_output_file.good();
          })) {
    std::puts("WRITE_SUCCESS");
  }
  ofs_output_file.close();

  std::ifstream ifs_input_file("example1.data");
  std::vector<std::string> result;
  if (auto optional_result{picklejar::read_vector_deep_copy(
          result, ifs_input_file, [](auto &_result, auto &char_buffer) {
            _result.emplace_back(std::begin(char_buffer),
                                 std::end(char_buffer));
          })}) {
    std::puts(("fifth element=" + optional_result.value().at(4)).c_str());
  }
  ifs_input_file.close();

  hexer::print_vec(result);
}

static void exampleSolution1dStream() {
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",   "16",
                                      "32", "64", "128", "256", "512", "1024"};

  std::ofstream ofs_output_file("example1.data");
  if (picklejar::deep_copy_vector_to_stream(
          string_vec, ofs_output_file,
          [](auto &string) { return string.size(); },
          [](auto &_ofs_output_file, auto &object, size_t element_size) {
            _ofs_output_file.write(object.data(),
                                   std::streamsize(element_size));
            return _ofs_output_file.good();
          })) {
    std::puts("WRITE_SUCCESS");
  }
  ofs_output_file.close();

  std::ifstream ifs_input_file("example1.data");
  std::vector<std::string> result;
  if (auto optional_result{picklejar::deep_read_vector_from_stream(
          result, ifs_input_file, [](auto &_result, auto &char_buffer) {
            _result.emplace_back(std::begin(char_buffer),
                                 std::end(char_buffer));
          })}) {
    std::puts(("fifth element=" + optional_result.value().at(4)).c_str());
  }
  ifs_input_file.close();

  hexer::print_vec(result);
}

static void exampleSolution1dFile() {
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",   "16",
                                      "32", "64", "128", "256", "512", "1024"};

  if (picklejar::deep_copy_vector_to_file(
          string_vec, "example1.data",
          [](auto &string) { return string.size(); },
          [](auto &_ofs_output_file, auto &object, size_t element_size) {
            _ofs_output_file.write(object.data(),
                                   std::streamsize(element_size));
            return _ofs_output_file.good();
          })) {
    std::puts("WRITE_SUCCESS");
  }

  std::vector<std::string> result;
  if (auto optional_result{picklejar::deep_read_vector_from_file(
          result, "example1.data", [](auto &_result, auto &char_buffer) {
            _result.emplace_back(std::begin(char_buffer),
                                 std::end(char_buffer));
          })}) {
    std::puts(("fifth element=" + optional_result.value().at(4)).c_str());
  }

  hexer::print_vec(result);
}

static void exampleSolution1dBuffer() {
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",   "16",
                                      "32", "64", "128", "256", "512", "1024"};
  if (auto optional_vector_byte_buffer = picklejar::deep_copy_vector_to_buffer(
          string_vec, [](auto &string) { return string.size(); },
          [](picklejar::ByteVectorWithCounter &_buffer,
             const std::string &string, size_t element_size) {
            _buffer.write(string.data(), element_size);
            return true;
          })) {
    std::puts("WRITE_SUCCESS");

    std::vector<std::string> result;
    optional_vector_byte_buffer.value().byte_counter =
        0;  // reset counter so we can read from same buffer
    if (auto optional_result{picklejar::deep_read_vector_from_buffer(
            result, optional_vector_byte_buffer.value(),
            [](std::vector<std::string> &_result,
               std::vector<char> &byte_buffer) {
              _result.emplace_back(std::begin(byte_buffer),
                                   std::end(byte_buffer));
            })}) {
      std::puts(("fifth element=" + optional_result.value().at(0)).c_str());
      hexer::print_vec(result);
    }
  }
}

static void exampleSolution2() {
  std::vector<std::string> string_vec{
      "string1", "string2", "string3", "string4",  "string5", "string6",
      "string7", "string8", "string9", "string10", "string11"};

  if (picklejar::write_vector_to_file(string_vec, "example1.data"))
    std::puts("WRITESUCCESS");

  if (auto optional_read_vector = picklejar::read_vector_from_file<std::string>(
          "example1.data",
          [count = 0](auto &blank_instance,
                      auto &valid_bytes_from_new_blank_instance,
                      auto &bytes_from_file) mutable {
            picklejar::util::preserve_blank_instance_member(
                0, sizeof(std::string), valid_bytes_from_new_blank_instance,
                bytes_from_file);
            picklejar::util::copy_new_bytes_to_instance(
                valid_bytes_from_new_blank_instance, blank_instance,
                sizeof(std::string));
            blank_instance = "string" + std::to_string(++count);
          });
      optional_read_vector.has_value()) {
    std::puts(("READSUCCESS: last_element=" +
               optional_read_vector.value().at(
                   optional_read_vector.value().size() - 1))
                  .c_str());
    hexer::print_vec(optional_read_vector.value());
  }
}

static void exampleSolution3() {
  std::vector<std::string> string_vec{
      "string1", "string2", "string3", "string4",  "string5", "string6",
      "string7", "string8", "string9", "string10", "string11"};

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
}

static void exampleSolution4() {
  std::vector<std::string> string_vec{
      "string1", "string2", "string3", "string4",  "string5", "string6",
      "string7", "string8", "string9", "string10", "string11"};

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
          });
      optional_read_vector.has_value()) {
    std::puts(
        ("READSUCCESS: fifth_element=" + optional_read_vector.value().at(4))
            .c_str());
  }
}

auto main() -> int {
  // example1();
  // example2a();
  // example2b();
  // exampleSolution1a();
  // exampleSolution1b();
  // exampleSolution1c();
  exampleSolution1dStream();
  exampleSolution1dBuffer();
  exampleSolution1dFile();
  // exampleSolution2();
  // exampleSolution3();
  // exampleSolution4();
  std::terminate();
  // STRING EXAMPLE
  std::vector<std::string> string_vec{"",   "1",  "2",   "4",   "8",  "16",
                                      "32", "64", "128", "256", "512"};

  if (picklejar::write_vector_to_file(string_vec, "example1.data"))
    std::puts("WRITESUCCESS");

  if (auto optional_read_vector = picklejar::read_vector_from_file<std::string>(
          "example1.data",
          [count = 1](auto &blank_instance,
                      auto &valid_bytes_from_new_blank_instance,
                      auto &bytes_from_file) mutable {
            picklejar::util::preserve_blank_instance_member(
                0, sizeof(std::string), valid_bytes_from_new_blank_instance,
                bytes_from_file);
            picklejar::util::copy_new_bytes_to_instance(
                valid_bytes_from_new_blank_instance, blank_instance,
                sizeof(std::string));
            blank_instance = "string" + std::to_string(++count);
          });
      optional_read_vector.has_value()) {
    std::puts(("READSUCCESS: last_element=" +
               optional_read_vector.value().at(
                   optional_read_vector.value().size() - 1))
                  .c_str());
    hexer::print_vec(optional_read_vector.value());
  }

  std::vector<int> int_vec{0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

  if (picklejar::write_vector_to_file(int_vec, "example1.data"))
    std::puts("WRITESUCCESS");

  if (auto optional_read_vector =
          picklejar::read_vector_from_file<int>("example1.data");
      optional_read_vector) {
    std::puts(("READSUCCESS: last element:" +
               std::to_string(optional_read_vector.value().at(
                   optional_read_vector.value().size() - 1)))
                  .c_str());
    hexer::print_vec(optional_read_vector.value());
  }

  std::vector<ComplexStructure> vec{};
  vec.reserve(4);
  vec.emplace_back("1h");
  vec.at(0).note_range_selector.byte2_note_range_start = 16 * 16 * 16 * 16;
  vec.at(0).midi_channel = 8738;
  vec.emplace_back("2");
  vec.at(1).note_range_selector.byte2_note_range_start = 2;
  vec.at(1).midi_channel = 8738;
  vec.emplace_back("3");
  vec.at(2).note_range_selector.byte2_note_range_start = 3;
  vec.at(2).midi_channel = 8738;
  vec.emplace_back("4");
  vec.at(3).note_range_selector.byte2_note_range_start = 4;
  vec.at(3).midi_channel = 8738;

  if (picklejar::write_vector_to_file(vec, "example1.data"))
    std::puts("WRITESUCCESS");
  print_vec(vec);

  // BUFFER VERSION
  std::vector<char> buffer_vector_copy_test{
      picklejar::write_vector_to_buffer(vec)};

  std::vector<ComplexStructure> buff_vec{};
  buff_vec.reserve(vec.size());
  if (auto optional_read_vector =
          picklejar::read_vector_from_buffer<ComplexStructure>(
              buff_vec, buffer_vector_copy_test,
              [count = 1](auto &blank_instance,
                          auto &valid_bytes_from_new_blank_instance,
                          auto &bytes_from_file) mutable {
                constexpr auto class_member_offset =
                    offsetof(ComplexStructure, id);

                hexer::print_address_range_as_hex_unchecked(
                    bytes_from_file, sizeof(ComplexStructure),
                    class_member_offset, sizeof(std::string));
                hexer::print_address_range_as_hex_unchecked(
                    valid_bytes_from_new_blank_instance,
                    sizeof(ComplexStructure), class_member_offset,
                    sizeof(std::string));
                picklejar::util::preserve_blank_instance_member(
                    class_member_offset, sizeof(std::string),
                    valid_bytes_from_new_blank_instance, bytes_from_file);
                picklejar::util::copy_new_bytes_to_instance(
                    bytes_from_file, blank_instance, sizeof(ComplexStructure));
                // blank_instance.id = "modified" + std::to_string(++count);
              },
              [count = 0]() mutable {
                ++count;
                return std::tuple("firstconstructor" + std::to_string(count),
                                  "secondconstructor" + std::to_string(count));
              });
      optional_read_vector.has_value()) {
    std::puts(("READSUCCESS" +
               std::to_string(optional_read_vector.value().at(0).midi_channel))
                  .c_str());
    print_vec(optional_read_vector.value());
  }
  // BROKEN VERSION because of string present in struct
  /*if (auto optional_read_vector =
          picklejar::read_vector_from_file<ComplexStructure>("example1.data");
      optional_read_vector) {
    std::puts(("READSUCCESS" +
               std::to_string(optional_read_vector.value().at(0).midi_channel))
                  .c_str());
                  }*/

  // FINAL VERSION
  if (auto optional_read_vector =
          picklejar::read_vector_from_file<ComplexStructure>(
              "example1.data",
              [count = 1](auto &blank_instance,
                          auto &valid_bytes_from_new_blank_instance,
                          auto &bytes_from_file) mutable {
                constexpr auto class_member_offset =
                    offsetof(ComplexStructure, id);
                picklejar::util::preserve_blank_instance_member(
                    class_member_offset, sizeof(std::string),
                    valid_bytes_from_new_blank_instance, bytes_from_file);
                picklejar::util::copy_new_bytes_to_instance(
                    bytes_from_file, blank_instance, sizeof(ComplexStructure));
                // blank_instance.id = "hello" + std::to_string(++count);
              },
              [count = 0]() mutable {
                ++count;
                return std::tuple("hello" + std::to_string(count),
                                  "colo" + std::to_string(count));
              });
      optional_read_vector) {
    std::puts(("READSUCCESS" +
               std::to_string(optional_read_vector.value().at(0).midi_channel))
                  .c_str());
    print_vec(optional_read_vector.value());
  }
  // TEST VERSION 2 without asserts, with helper functions like FINAL VERSION
  // but with printhex statements
  /*if (auto optional_read_vector =
    picklejar::read_vector_from_file<ComplexStructure>( "example1.data", [count
    = 1](auto &blank_instance, auto &valid_bytes_from_new_blank_instance, auto
    &bytes_from_file) mutable { constexpr auto class_member_offset =
                offsetof(ComplexStructure, id);
            print_as_hex(bytes_from_file, sizeof(ComplexStructure),
                         class_member_offset, sizeof(std::string));
            print_as_hex(valid_bytes_from_new_blank_instance,
                         sizeof(ComplexStructure), class_member_offset,
                         sizeof(std::string));
            picklejar::util::preserve_blank_instance_member(
                class_member_offset, sizeof(std::string),
                valid_bytes_from_new_blank_instance, bytes_from_file);
            picklejar::util::copy_new_bytes_to_instance(bytes_from_file,
    blank_instance, sizeof(ComplexStructure)); print_as_hex(blank_instance,
    sizeof(ComplexStructure), class_member_offset, sizeof(std::string));
             blank_instance.id = std::to_string(++count);
          });
      optional_read_vector) {
    std::puts(("READSUCCESS" +
               std::to_string(optional_read_vector.value().at(0).midi_channel))
                  .c_str());
    print_vec(optional_read_vector.value());
    }*/

  /* TEST VERSION 1 with asserts
  if (auto optional_read_vector =
  picklejar::read_vector_from_file<ComplexStructure>( "example1.data", [count =
  1](auto &blank_instance, auto &valid_bytes_from_new_blank_instance, auto
  &bytes_from_file) mutable { auto offset = reinterpret_cast<char
  *>(&blank_instance.id) - reinterpret_cast<char *>(&blank_instance);
            assert(offsetof(ComplexStructure, id) == offset);
            assert(reinterpret_cast<char *>(&blank_instance) + offset ==
                   reinterpret_cast<char *>(&blank_instance.id));
            std::puts(("offset" + std::to_string(offset)).c_str());
            print_as_hex(&bytes_from_file, sizeof(ComplexStructure), offset,
                         sizeof(std::string));
            print_as_hex(&valid_bytes_from_new_blank_instance,
                         sizeof(ComplexStructure), offset,
                         sizeof(std::string));
            // copy valid std::string bytes to the file bytes that will be
            // copied to the blank instance
            std::memcpy(&bytes_from_file + offset,
                        &valid_bytes_from_new_blank_instance + offset,
                        sizeof(std::string));
            // copy our new bytes to our blank copy
            std::memcpy(&blank_instance, &bytes_from_file,
                        sizeof(ComplexStructure));
            print_as_hex(&blank_instance, sizeof(ComplexStructure), offset,
                         sizeof(std::string));
            blank_instance.id = std::to_string(++count);
          });
      optional_read_vector) {
    std::puts(("READSUCCESS" +
               std::to_string(optional_read_vector.value().at(0).midi_channel))
                  .c_str());
                  }*/
}
