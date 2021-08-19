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

#include "../picklejar.hpp"
#include "../tests/hexer/hexer.hpp"

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
  // THIS SHOULD NOT work because strings need to be deep copied
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
                bytes_from_file, blank_instance, sizeof(std::string));
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
        if (picklejar::basic_stream_write(ofs_output_file, object.data(),
                                          object.size())) {
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

/* THIS EXAMPLE HAS BEEN MADE INTO FUNCTIONS inside the picklejar library see
exampleSolution1dStream() static auto store_object(auto &object, size_t
object_size, std::ofstream &ofs_output_file, auto &&element_write_lambda) ->
bool { if (picklejar::write_object_to_stream(object_size, ofs_output_file)) {
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
*/
static void exampleSolution1c() {
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",   "16",
                                      "32", "64", "128", "256", "512", "1024"};

  std::ofstream ofs_output_file("example1.data");
  if (picklejar::write_vector_deep_copy(
          string_vec, ofs_output_file,
          [](auto &string) { return string.size(); },
          [](auto &_ofs_output_file, auto &object, size_t element_size) {
            return picklejar::basic_stream_write(_ofs_output_file,
                                                 object.data(), element_size);
          })) {
    std::puts("WRITE_SUCCESS");
  }
  ofs_output_file.close();

  std::ifstream ifs_input_file("example1.data");
  std::vector<std::string> result;
  if (auto optional_result{picklejar::read_vector_deep_copy(
          result, ifs_input_file,
          [](auto &_result, auto &byte_vector_with_counter) {
            _result.emplace_back(std::begin(byte_vector_with_counter),
                                 std::end(byte_vector_with_counter));
            return true;
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
            return picklejar::basic_stream_write(_ofs_output_file,
                                                 object.data(), element_size);
          })) {
    std::puts("WRITE_SUCCESS");
  }
  ofs_output_file.close();

  std::ifstream ifs_input_file("example1.data");
  std::vector<std::string> result;
  if (auto optional_result{picklejar::deep_read_vector_from_stream(
          result, ifs_input_file,
          [](auto &_result, auto &byte_vector_with_counter) {
            _result.emplace_back(std::begin(byte_vector_with_counter),
                                 std::end(byte_vector_with_counter));
            return true;
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
            return picklejar::basic_stream_write(_ofs_output_file,
                                                 object.data(), element_size);
          })) {
    std::puts("WRITE_SUCCESS");
  }

  std::vector<std::string> result;
  if (auto optional_result{picklejar::deep_read_vector_from_file(
          result, "example1.data",
          [](auto &_result, auto &byte_vector_with_counter) {
            _result.emplace_back(std::begin(byte_vector_with_counter),
                                 std::end(byte_vector_with_counter));
            return true;
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
    optional_vector_byte_buffer.value().set_counter(
        0);  // reset counter so we can read from same buffer
    if (auto optional_result{picklejar::deep_read_vector_from_buffer(
            result, optional_vector_byte_buffer.value(),
            [](std::vector<std::string> &_result, auto &byte_buffer) {
              _result.emplace_back(std::begin(byte_buffer),
                                   std::end(byte_buffer));
              return true;
            })}) {
      std::puts(("fifth element=" + optional_result.value().at(0)).c_str());
      hexer::print_vec(result);
    }
  }
}

static void exampleSolution1eFile() {
  struct IntBasedString {
    int id;
    std::string rand_str_id;
    IntBasedString() = default;
    explicit IntBasedString(int _id)
        : id(_id), rand_str_id("ID=" + std::to_string(std::rand())) {
      std::puts((std::to_string(_id) + " with " + rand_str_id + " Constructed")
                    .c_str());
    }
    explicit IntBasedString(int _id, const std::string _pretty_id)
        : id(_id), rand_str_id(_pretty_id) {}
  };
  std::vector<IntBasedString> intbased_vec(10);
  std::generate(std::begin(intbased_vec), std::end(intbased_vec),
                [count = 0]() mutable { return IntBasedString{++count}; });

  if (picklejar::deep_copy_vector_to_file(
          intbased_vec, "example1.data",
          [](const IntBasedString &object) {
            return sizeof(IntBasedString::id) + object.rand_str_id.size();
          },
          [](auto &_ofs_output_file, const IntBasedString &object,
             size_t element_size) {
            // write the id
            if (!picklejar::write_object_to_stream(object.id,
                                                   _ofs_output_file)) {
              return false;
            }
            // wee need to manually write it for the rand_str_id because a
            // string can have variable size
            return picklejar::basic_stream_write(_ofs_output_file,
                                                 object.rand_str_id.data(),
                                                 object.rand_str_id.size());
          })) {
    std::puts("WRITE_SUCCESS");
  } else {
    std::puts("WRITE_ERROR");
  }

  std::vector<IntBasedString> result;
  if (auto optional_result{picklejar::deep_read_vector_from_file(
          result, "example1.data",
          [](auto &_result,
             picklejar::ByteVectorWithCounter &byte_vector_with_counter) {
            auto optional_id = byte_vector_with_counter.read<int>();
            if (!optional_id) return false;
            std::string _pretty_id(
                std::begin(byte_vector_with_counter) +
                    int(byte_vector_with_counter.byte_counter.value()),
                std::end(byte_vector_with_counter));

            _result.emplace_back(optional_id.value(), _pretty_id);
            return true;
          })}) {
    std::puts(
        ("fifth element_id=" + std::to_string(optional_result.value().at(4).id))
            .c_str());
    std::puts(
        ("fifth element_rand_id=" + optional_result.value().at(4).rand_str_id)
            .c_str());
  }
}

static void exampleSolution1eFileStructChange() {
  struct IntBasedString {
    int id;
    std::string rand_str_id;
    IntBasedString() = default;
    explicit IntBasedString(int _id)
        : id(_id), rand_str_id("ID=" + std::to_string(std::rand())) {
      std::puts((std::to_string(_id) + " with " + rand_str_id + " Constructed")
                    .c_str());
    }
    explicit IntBasedString(int _id, const std::string _pretty_id)
        : id(_id), rand_str_id(_pretty_id) {}
  };
  std::vector<IntBasedString> intbased_vec(10);
  std::generate(std::begin(intbased_vec), std::end(intbased_vec),
                [count = 0]() mutable { return IntBasedString{++count}; });

  if (picklejar::deep_copy_vector_to_file<1>(
          intbased_vec, "example1.data",
          [](const IntBasedString &object) {
            return sizeof(IntBasedString::id) + object.rand_str_id.size();
          },
          [](auto &_ofs_output_file, const IntBasedString &object,
             size_t element_size) {
            // write the id
            if (!picklejar::write_object_to_stream(object.id,
                                                   _ofs_output_file)) {
              return false;
            }
            // wee need to manually write it for the rand_str_id because a
            // string can have variable size
            return picklejar::basic_stream_write(_ofs_output_file,
                                                 object.rand_str_id.data(),
                                                 object.rand_str_id.size());
          })) {
    std::puts("WRITE_SUCCESS");
  } else {
    std::puts("WRITE_ERROR");
  }

  std::vector<IntBasedString> result;
  if (auto optional_result{picklejar::deep_read_vector_from_file<1>(
          result, "example1.data",
          [](auto &_result,
             picklejar::ByteVectorWithCounter &byte_vector_with_counter) {
            size_t size_read = size_t{0};
            auto optional_id = byte_vector_with_counter.read<int>();
            if (!optional_id) return false;
            std::string _pretty_id(
                std::begin(byte_vector_with_counter) + size_read,
                std::end(byte_vector_with_counter));

            _result.emplace_back(optional_id.value(), _pretty_id);
            return true;
          })}) {
    std::puts(
        ("fifth element_id=" + std::to_string(optional_result.value().at(4).id))
            .c_str());
    std::puts(
        ("fifth element_rand_id=" + optional_result.value().at(4).rand_str_id)
            .c_str());
  }

  std::puts(
      "This section simulates what we would do if we wanted to update the "
      "struct:");
  // now let's say we change our struct, we will have to use the old read
  // function to make it compatible with the new one
  struct IntBasedStringChanged {
    std::string rand_str_id;
    int id;
    std::vector<std::pair<double, double>> new_important_pair_vector;
    IntBasedStringChanged() = default;
    explicit IntBasedStringChanged(int _id)
        : id(_id),
          rand_str_id("ID=" + std::to_string(std::rand())),
          new_important_pair_vector{{.9, .2}, {.2, .9}} {
      std::puts((std::to_string(_id) + " with " + rand_str_id + " Constructed")
                    .c_str());
    }
    explicit IntBasedStringChanged(
        int _id, const std::string _pretty_id,
        std::vector<std::pair<double, double>> _new_important_pair_vector)
        : id(_id),
          rand_str_id(_pretty_id),
          new_important_pair_vector{_new_important_pair_vector} {}
  };

  // First we have to read using the version <1> but modified to use our new 3
  // parameter constructor and then write using the version <2> which needs to
  // be changed to write things in the correct order
  std::vector<IntBasedStringChanged> result_changed;
  if (auto optional_result{picklejar::deep_read_vector_from_file<1>(
          result_changed, "example1.data",
          [new_elementgenerator = 1.0](auto &_result,
                                       picklejar::ByteVectorWithCounter
                                           &byte_vector_with_counter) mutable {
            auto optional_id = byte_vector_with_counter.read<int>();
            if (!optional_id) return false;
            std::string _pretty_id(
                std::begin(byte_vector_with_counter) +
                    int(byte_vector_with_counter.byte_counter.value()),
                std::end(byte_vector_with_counter));
            // we added a new member so we need to generate it here
            std::vector<std::pair<double, double>> _new_important_pair_vector{
                {1.0, .3 * new_elementgenerator},
                {0.3, 1.0 * ++new_elementgenerator}};
            _result.emplace_back(optional_id.value(), _pretty_id,
                                 _new_important_pair_vector);
            return true;
          })}) {
  }
  std::puts(
      ("fifth element_id=" + std::to_string(result_changed.at(4).id)).c_str());
  std::puts(
      ("fifth element_rand_id=" + result_changed.at(4).rand_str_id).c_str());
  std::puts("TEST");
#if 1
  if (picklejar::deep_copy_vector_to_file<2>(
          result_changed, "example1.data",
          [](const IntBasedStringChanged &object) {
            return sizeof(IntBasedString::id) + object.rand_str_id.size();
          },
          [](auto &_ofs_output_file, const IntBasedStringChanged &object,
             size_t element_size) {
            // write the id
            if (!picklejar::write_object_to_stream(object.id,
                                                   _ofs_output_file)) {
              return false;
            }
            // wee need to manually write it for the rand_str_id because a
            // string can have variable size
            return picklejar::basic_stream_write(_ofs_output_file,
                                                 object.rand_str_id.data(),
                                                 object.rand_str_id.size());
          })) {
    std::puts("WRITE_SUCCESS");
  } else {
    std::puts("WRITE_ERROR");
  }

  std::vector<IntBasedStringChanged> result_changed_v2;
  if (auto optional_result{picklejar::deep_read_vector_from_file<2>(
          result_changed_v2, "example1.data",
          [new_elementgenerator = 1.0](auto &_result,
                                       picklejar::ByteVectorWithCounter
                                           &byte_vector_with_counter) mutable {
            auto optional_id = byte_vector_with_counter.read<int>();
	    if(!optional_id) return false;
            std::string _pretty_id(
                std::begin(byte_vector_with_counter) +
                    int(byte_vector_with_counter.byte_counter.value()),
                std::end(byte_vector_with_counter));
            // we added a new member so we need to generate it here
            std::vector<std::pair<double, double>> _new_important_pair_vector{
                {1.0, .3 * new_elementgenerator},
                {0.3, 1.0 * ++new_elementgenerator}};
            _result.emplace_back(optional_id.value(), _pretty_id, _new_important_pair_vector);
	    return true;
          })}) {
    std::puts(("fifth element_id=" + std::to_string(result_changed.at(4).id))
                  .c_str());
    std::puts(
        ("fifth element_rand_id=" + result_changed.at(4).rand_str_id).c_str());
  }
#endif
#if 0
  // we make version 2 of our write function
  if (picklejar::deep_copy_vector_to_file<2>(
          result_changed, "example1.data",
          [](const IntBasedStringChanged &object) {
            return /*size of size_t to store our string size*/ sizeof(size_t) +
                   /*the size of our string*/ object.rand_str_id.size() +
                   /*the size of our id*/ sizeof(int) +
                   /*size of our vector pair of doubles*/
                   object.new_important_pair_vector.size() *
                       (sizeof(std::pair<double, double>));
          },
          [](auto &_ofs_output_file, const IntBasedStringChanged &object,
             size_t element_size) {
            // we have changed it to first write the string data and then the
            // int id
            std::puts(std::to_string(object.rand_str_id.size()).c_str());
	    std::puts(object.rand_str_id.c_str());
            if (!picklejar::write_object_to_stream(object.rand_str_id.size(),
                                                   _ofs_output_file))
              return false;
            if (!picklejar::basic_stream_write(_ofs_output_file,
                                               object.rand_str_id.data(),
                                               object.rand_str_id.size()))
              return false;

            // write the id
            if (!picklejar::write_object_to_stream(object.id, _ofs_output_file))
              return false;
            // write the new element we just added in this new version
            if (!picklejar::write_vector_to_stream(
                    object.new_important_pair_vector, _ofs_output_file))
              return false;
            return true;
          })) {
    std::puts("WRITE_SUCCESS");
  } else {
    std::puts("WRITE_ERROR");
  }

  std::vector<IntBasedStringChanged> result_changed_v2;
  if (auto optional_result{picklejar::deep_read_vector_from_file<2>(
          result_changed_v2, "example1.data",
          [](auto &_result,
             picklejar::ByteVectorWithCounter &byte_vector_with_counter) {
            const size_t size_of_id = sizeof(IntBasedStringChanged::id);
            auto optional_size_of_string =
                byte_vector_with_counter.read<size_t>();
            if (!optional_size_of_string) return false;
            std::string _pretty_id(
                std::begin(byte_vector_with_counter),
                std::begin(byte_vector_with_counter) +
                    int(optional_size_of_string.value()));
            std::puts((std::to_string(optional_size_of_string.value()) + " " +
                       std::to_string(_pretty_id.size()))
                          .c_str());
            std::puts(_pretty_id.c_str());
            // advance byte_count by size of the string
            byte_vector_with_counter.byte_counter.value() +=
                optional_size_of_string.value();
            auto optional_id = byte_vector_with_counter.read<int>();
            if (!optional_id) return false;

            std::vector<std::pair<double, double>> _new_important_pair_vector{
                {1.0, .3}, {0.3, 1.0}};
            _result.emplace_back(0, _pretty_id, _new_important_pair_vector);
            return true;
          })}) {
    std::puts(
        ("fifth element_id=" + std::to_string(optional_result.value().at(0).id))
            .c_str());
    std::puts(
        ("fifth element_rand_id=" + optional_result.value().at(0).rand_str_id)
            .c_str());
  } else {
    std::puts("READ_ERROR");
  }
#endif
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
            // we are passed a blank_instance of a std::string which will be
            // added to our vector. We don't need to do anything with the other
            // parameters since a string is a very simple object and we don't
            // need the bytes from the file to generate this
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
            // we don't have to do anything here since we have constructed our
            // string directly. We no longer modify the string in this lambda
          },
          [count = 0]() mutable {
            // we return a tuple with the parameters used to construct our
            // non-trivially copiable type. This will be useful for more
            // complex types, like structs that have strings in them.
            // We made the lambda mutable to be able to use a counter
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
             auto &bytes_from_file) {});
      optional_read_vector.has_value()) {
    std::puts(("READSUCCESS: fifth_element(should be empty string)=" +
               optional_read_vector.value().at(4))
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
  // exampleSolution1dStream();
  // exampleSolution1dBuffer();
  // exampleSolution1dFile();
  // exampleSolution2();
  // exampleSolution3();
  // exampleSolution4();
  // exampleSolution1eFile();
  exampleSolution1eFileStructChange();

  std::terminate();

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
                return std::make_tuple("hello" + std::to_string(count),
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
