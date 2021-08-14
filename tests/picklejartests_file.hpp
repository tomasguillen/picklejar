#include <boost/ut.hpp>
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
#include <hexer.hpp>

#include "../picklejar.hpp"
#include "picklejartests_teststructures.hpp"

using namespace boost::ut;

void picklejartests_file() {
  // namespace u = boost::ut;
  // using namespace boost::ut::literals;
  // using namespace boost::ut::operators::terse;
  auto &&preserve_constructed_id_in_our_new_copy =
      [](auto &blank_instance, auto &valid_bytes_from_new_blank_instance,
         auto &bytes_from_file) mutable {
        constexpr auto class_member_offset = offsetof(TestStructure, id);

        hexer::print_address_range_as_hex_unchecked(bytes_from_file, sizeof(TestStructure),
                     class_member_offset, sizeof(std::string));
        hexer::print_address_range_as_hex_unchecked(valid_bytes_from_new_blank_instance, sizeof(TestStructure),
                     class_member_offset, sizeof(std::string));
        picklejar::util::preserve_blank_instance_member(
            class_member_offset, sizeof(std::string),
            valid_bytes_from_new_blank_instance, bytes_from_file);
        picklejar::util::copy_new_bytes_to_instance(
            bytes_from_file, blank_instance, sizeof(TestStructure));

        hexer::print_object_as_hex(blank_instance, class_member_offset,
                     sizeof(std::string));
      };
  auto &&preserve_constructed_id_in_our_new_copy_and_modify_it =
      [preserve_constructed_id_in_our_new_copy, count = 1](
          auto &blank_instance, auto &valid_bytes_from_new_blank_instance,
          auto &bytes_from_file) mutable {
        preserve_constructed_id_in_our_new_copy(
            blank_instance, valid_bytes_from_new_blank_instance,
            bytes_from_file);
        blank_instance.id = "modified" + std::to_string(++count);
      };
  auto &&constructor_generator_one_param = [count = 0]() mutable {
    ++count;
    return std::tuple("firstconstructor" + std::to_string(count));
  };
  auto &&constructor_generator_two_params = [count = 0]() mutable {
    ++count;
    return std::tuple("firstconstructor" + std::to_string(count),
                      "secondconstructor" + std::to_string(count));
  };
  // prepare vector for tests
  auto &&prepare_teststructure_vector_for_tests =
      []() -> std::vector<TestStructure> {
    std::vector<TestStructure> struct_vec{};
    struct_vec.reserve(4);
    struct_vec.emplace_back("original1");
    struct_vec.at(0).note_range_selector.byte2_note_range_start =
        16 * 16 * 16 * 16;
    struct_vec.at(0).midi_channel = 8738;
    struct_vec.emplace_back("original2");
    struct_vec.at(1).note_range_selector.byte2_note_range_start = 2;
    struct_vec.at(1).midi_channel = 8738;
    struct_vec.emplace_back("original3");
    struct_vec.at(2).note_range_selector.byte2_note_range_start = 3;
    struct_vec.at(2).midi_channel = 8738;
    struct_vec.emplace_back("original4");
    struct_vec.at(3).note_range_selector.byte2_note_range_start = 4;
    struct_vec.at(3).midi_channel = 8738;
    return struct_vec;
  };

  auto &&do_buffer_test = [&](std::string test_id, auto &&write_function,
                              auto &&read_function,
                              const std::string &expected_modification,
                              auto &&prepare_struct_vector_for_tests) {
    test_id += expected_modification;
    auto struct_vec = prepare_struct_vector_for_tests();

    std::string file_name{"filetests.generated_test_data"};
    expect(true == picklejar::write_vector_to_file(struct_vec, file_name))
        << "Failed to write to file";
    auto buffer_vector_copy_test = write_function(struct_vec);
    auto optional_read_vector = read_function(file_name);

    expect(true == optional_read_vector.has_value())
        << test_id << " "
        << "picklejar::read_vector_from_file<TestStructure>() failed";

    expect(true == std::equal(std::begin(optional_read_vector.value()),
                              std::end(optional_read_vector.value()),
                              std::begin(struct_vec)))
        << test_id << " "
        << "Read vector integer components are not equal to vector used to "
           "test";

    expect(false ==
           std::equal(
               std::begin(optional_read_vector.value()),
               std::end(optional_read_vector.value()), std::begin(struct_vec),
               [](const auto &x, const auto &y) { return x.id == y.id; }))
        << test_id << " "
        << "Read vector id is equal to vector used to test, but it should have "
           "been modified";

    expect(true == std::all_of(std::begin(optional_read_vector.value()),
                               std::end(optional_read_vector.value()),
                               [&expected_modification](const auto &s) {
                                 return s.id.starts_with(expected_modification);
                               }))
        << test_id << " "
        << "Read vector not modified as expected";

    if (optional_read_vector.has_value()) {
      print_vec(optional_read_vector.value());
    }
  };

  auto &&do_file_v1_test = [&](std::string test_id, auto &&write_function,
                               auto &&read_function,
                               const std::string &expected_modification,
                               auto &&prepare_struct_vector_for_tests) {
    test_id += expected_modification;
    auto struct_vec = prepare_struct_vector_for_tests();

    std::string file_name{"filetests.generated_test_data"};
    expect(true == picklejar::write_vector_to_file(struct_vec, file_name))
        << "Failed to write to file";
    auto buffer_vector_copy_test = write_function(struct_vec);
    auto optional_read_vector = read_function(file_name);

    expect(true == optional_read_vector.has_value())
        << test_id << " "
        << "picklejar::read_vector_from_file<TestStructure>() failed";

    expect(true == std::equal(std::begin(optional_read_vector.value()),
                              std::end(optional_read_vector.value()),
                              std::begin(struct_vec)))
        << test_id << " "
        << "Read vector integer components are not equal to vector used to "
           "test";

    expect(true == std::equal(std::begin(optional_read_vector.value()),
                              std::end(optional_read_vector.value()),
                              std::begin(struct_vec),
                              [](const auto &x, const auto &y) {
                                return x.byte2_note_range_end ==
                                       y.byte2_note_range_end;
                              }))
        << test_id << " "
        << "Read vector id is equal to vector used to test, but it should have "
           "been modified";

    if (optional_read_vector.has_value()) {
      print_vec(optional_read_vector.value());
    }
  };

  auto &&buffer_write_function = [](auto &struct_vec) -> bool { return true; };
  const std::string startswith_firstconstructor_expected_modification{
      "firstconstructor"};
  const std::string startswith_secondconstructor_expected_modification{
      "secondconstructor"};
  const std::string startswith_modified_expected_modification{"modified"};

  auto &&do_buffer_test_w_data = [&](std::string test_id, auto &&write_function,
                                     auto test_data,
                                     auto &&prepare_struct_vector_for_tests) {
    do_buffer_test(test_id, write_function, test_data.second, test_data.first,
                   prepare_struct_vector_for_tests);
  };
  auto &&do_file_v1_test_w_data = [&](std::string test_id,
                                      auto &&write_function, auto test_data,
                                      auto &&prepare_struct_vector_for_tests) {
    do_file_v1_test(test_id, write_function, test_data.second, test_data.first,
                    prepare_struct_vector_for_tests);
  };

  // FILE_V3
  auto &&file_v3_firstconstructor_read_function = [&](const auto &file_name) {
    return picklejar::read_vector_from_file<TestStructure>(
        file_name, preserve_constructed_id_in_our_new_copy,
        constructor_generator_one_param);
  };

  auto &&file_v3_secondconstructor_read_function = [&](const auto &file_name) {
    return picklejar::read_vector_from_file<TestStructure>(
        file_name, preserve_constructed_id_in_our_new_copy,
        constructor_generator_two_params);
  };

  auto &&file_v3_modified_read_function = [&](const auto &file_name) {
    return picklejar::read_vector_from_file<TestStructure>(
        file_name, preserve_constructed_id_in_our_new_copy_and_modify_it,
        constructor_generator_two_params);
  };
  auto file_v3_firstconstructor_test_data =
      make_pair(startswith_firstconstructor_expected_modification,
                file_v3_firstconstructor_read_function);
  auto file_v3_secondconstructor_test_data =
      make_pair(startswith_secondconstructor_expected_modification,
                file_v3_secondconstructor_read_function);

  auto file_v3_modified_test_data =
      make_pair(startswith_modified_expected_modification,
                file_v3_modified_read_function);

  "file_v3"_test = [&] {
    do_buffer_test_w_data("file_v3_", buffer_write_function,
                          file_v3_firstconstructor_test_data,
                          prepare_teststructure_vector_for_tests);
    do_buffer_test_w_data("file_v3_", buffer_write_function,
                          file_v3_secondconstructor_test_data,
                          prepare_teststructure_vector_for_tests);
    do_buffer_test_w_data("file_v3_", buffer_write_function,
                          file_v3_modified_test_data,
                          prepare_teststructure_vector_for_tests);
  };

  // FILE_V2
  auto &&file_v2_notmodified_read_function = [&](const auto &file_name) {
    return picklejar::read_vector_from_file<TestStructure>(
        file_name, preserve_constructed_id_in_our_new_copy);
  };

  auto &&file_v2_modified_read_function = [&](const auto &file_name) {
    return picklejar::read_vector_from_file<TestStructure>(
        file_name, preserve_constructed_id_in_our_new_copy_and_modify_it);
  };
  const std::string startswith_default_expected_modification{"default"};
  auto file_v2_notmodified_test_data =
      make_pair(startswith_default_expected_modification,  // expect default
                file_v2_notmodified_read_function);

  auto file_v2_modified_test_data =
      make_pair(startswith_modified_expected_modification,
                file_v2_modified_read_function);

  "file_v2"_test = [&] {
    do_buffer_test_w_data("file_v2_", buffer_write_function,
                          file_v2_notmodified_test_data,
                          prepare_teststructure_vector_for_tests);
    do_buffer_test_w_data("file_v2_", buffer_write_function,
                          file_v2_modified_test_data,
                          prepare_teststructure_vector_for_tests);
  };

  // FILE_V1

  auto &&file_v1_read_function = [&](const auto &file_name) {
    return picklejar::read_vector_from_file<TestStructure>(file_name);
  };

  auto file_v1_test_data =
      make_pair(startswith_default_expected_modification,  // expect default
                file_v1_read_function);
  "file_v1_not_triviallycopiable_structs_test"_test = [&] {
    expect(false == std::is_trivially_copyable_v<TestStructure>)
        << "TestStructure SHOULD NOT be trivially copiable";
    expect(true == std::is_trivially_copyable_v<TrivialStructure>)
        << "This struct SHOULD be trivially copiable";
  };
  // THE FOLLOWING 2 SHOULD FAIL TO COMPILE BECAUSE THEY ARENT TRIVIALLY
  // COPIABLE
  /*"file_v1"_test = [&] {
    do_buffer_test_w_data("file_v1_", buffer_write_function,
                          file_v1_test_data,
    prepare_teststructure_vector_for_tests);
                          };*/
  /*"object_file_v1"_test = [&] {
    TestStructure test_object{"original"};
    picklejar::ManagedAlignedStorageCopy<TestStructure> copy{};
    auto test_buffer{picklejar::write_object_to_buffer(test_object)};
    size_t bytes_read_so_far{};
    auto recovered_object = *(read_object_from_file<TestStructure>(
                                  copy, test_buffer, bytes_read_so_far))
                                 .get_pointer_to_copy();
                                 };*/

  auto &&file_v1_read_function_inner = [&](const auto &file_name) {
    return picklejar::read_vector_from_file<TrivialStructure>(file_name);
  };

  auto file_v1_test_data_innerstruct =
      make_pair(startswith_default_expected_modification,  // expect default
                file_v1_read_function_inner);
  auto prepare_triviallyconstructiblestruct_vector_for_tests =
      []() -> std::vector<TrivialStructure> {
    std::vector<TrivialStructure> struct_vec{};
    struct_vec.reserve(4);
    struct_vec.emplace_back();
    struct_vec.at(0).byte2_note_range_start = 1;
    struct_vec.at(0).byte2_note_range_end = 8738;
    struct_vec.emplace_back();
    struct_vec.at(1).byte2_note_range_start = 2;
    struct_vec.at(1).byte2_note_range_end = 8738;
    struct_vec.emplace_back();
    struct_vec.at(2).byte2_note_range_start = 3;
    struct_vec.at(2).byte2_note_range_end = 8738;
    struct_vec.emplace_back();
    struct_vec.at(3).byte2_note_range_start = 4;
    struct_vec.at(3).byte2_note_range_end = 8738;
    return struct_vec;
  };

  "file_v1_innerstruct"_test = [&] {
    do_file_v1_test_w_data(
        "file_v1_", buffer_write_function, file_v1_test_data_innerstruct,
        prepare_triviallyconstructiblestruct_vector_for_tests);
  };
  "object_file_v1_innerstruct"_test = [&] {
    TrivialStructure test_object{};
    expect(true == picklejar::write_object_to_file(
                       test_object, "filetests.generated_test_data"));
    auto recovered_object = picklejar::read_object_from_file<TrivialStructure>(
        "filetests.generated_test_data");
    expect(true == (recovered_object == test_object))
        << "test object not equal to recovered object";
  };

  auto &&test_teststructure = [&](std::string test_id, auto &recovered_object,
                                  auto &test_object,
                                  auto expected_modification) {
    test_id += expected_modification;
    expect(true == (recovered_object == test_object))
        << test_id << "test object not equal to recovered object";
    expect(true == (recovered_object.id.starts_with(expected_modification)))
        << test_id << "test object id not what we expected";
  };
  "object_file_v3"_test = [&] {
    const TestStructure test_object{};
    expect(true == picklejar::write_object_to_file(
                       test_object, "filetests.generated_test_data"));
    auto recovered_optional = picklejar::read_object_from_file<TestStructure>(
        "filetests.generated_test_data",
        preserve_constructed_id_in_our_new_copy,
        constructor_generator_one_param);
    expect(true == recovered_optional.has_value())
        << "optional must have value";
    auto recovered_object{std::move(recovered_optional.value())};
    test_teststructure("file_v3_", recovered_object, test_object,
                       startswith_firstconstructor_expected_modification);
    auto recovered_optional_2 = picklejar::read_object_from_file<TestStructure>(
        "filetests.generated_test_data",
        preserve_constructed_id_in_our_new_copy,
        constructor_generator_two_params);
    expect(true == recovered_optional_2.has_value())
        << "optional must have value";
    auto recovered_object_2{std::move(recovered_optional_2.value())};
    test_teststructure("file_v3_", recovered_object_2, test_object,
                       startswith_secondconstructor_expected_modification);
    auto recovered_optional_3 = picklejar::read_object_from_file<TestStructure>(
        "filetests.generated_test_data",
        preserve_constructed_id_in_our_new_copy_and_modify_it,
        constructor_generator_two_params);
    expect(true == recovered_optional_3.has_value())
        << "optional must have value";
    auto recovered_object_3{std::move(recovered_optional_3.value())};
    test_teststructure("file_v3_", recovered_object_3, test_object,
                       startswith_modified_expected_modification);
  };

  "object_file_v2"_test = [&] {
    const TestStructure test_object{};
    expect(true == picklejar::write_object_to_file(
                       test_object, "filetests.generated_test_data"));
    auto recovered_optional = picklejar::read_object_from_file<TestStructure>(
        "filetests.generated_test_data",
        preserve_constructed_id_in_our_new_copy);
    expect(true == recovered_optional.has_value())
        << "recovered optional must have a value";
    auto recovered_object{std::move(recovered_optional.value())};
    test_teststructure("file_v2_", recovered_object, test_object,
                       startswith_default_expected_modification);
    auto recovered_optional_2 = picklejar::read_object_from_file<TestStructure>(
        "filetests.generated_test_data",
        preserve_constructed_id_in_our_new_copy_and_modify_it);
    expect(true == recovered_optional_2.has_value())
        << "recovered optional must have a value";
    auto recovered_object_2{std::move(recovered_optional_2.value())};
    test_teststructure("file_v2_", recovered_object_2, test_object,
                       startswith_modified_expected_modification);
  };

  // STREAM REMAINING COVERAGE
  "object_stream_v3"_test = [&] {
    const TestStructure test_object{};
    expect(true == picklejar::write_object_to_file(
                       test_object, "filetests.generated_test_data"));
    std::ifstream ifstream_input_file("filetests.generated_test_data",
                                      std::ios::in | std::ios::binary);
    // picklejar::ManagedAlignedStorageCopy<TestStructure> copy{};
    auto recovered_optional = picklejar::read_object_from_stream<TestStructure>(
        ifstream_input_file, preserve_constructed_id_in_our_new_copy,
        constructor_generator_one_param);
    expect(true == recovered_optional.has_value())
        << "recovered optional must have a value";
    auto recovered_object{std::move(recovered_optional.value())};
    test_teststructure("stream_v3_", recovered_object, test_object,
                       startswith_firstconstructor_expected_modification);
    ifstream_input_file.close();
  };

  "object_stream_v1_"_test = [&] {
    const TrivialStructure test_object{};
    expect(true == picklejar::write_object_to_file(
                       test_object, "filetests.generated_test_data"));
    std::ifstream ifstream_input_file("filetests.generated_test_data",
                                      std::ios::in | std::ios::binary);
    // expect (false == ifstream_input_file.fail()) << "expected false";
    auto recovered_optional =
        picklejar::read_object_from_stream<TrivialStructure>(
            ifstream_input_file);
    expect(true == recovered_optional.has_value())
        << "returned optional should have value";
    expect(true == (recovered_optional.value() == test_object))
        << "stream_v1_ new object is not equal to test_object";
    ifstream_input_file.close();
  };
  // Test Problem cases Ex: when file doesn't exist, etc...
  "object_file_problems_v1"_test = [&] {
    std::ifstream ifstream_input_file("filetests.nonexistent_file",
                                      std::ios::in | std::ios::binary);
    auto recovered_optional =
        picklejar::read_object_from_stream<TrivialStructure>(
            ifstream_input_file);
    expect(false == recovered_optional.has_value())
        << "returned optional SHOULD NOT have value";
    auto recovered_optional_2 =
        picklejar::read_object_from_file<TrivialStructure>(
            "filetests.nonexistent_file");
    expect(false == recovered_optional_2.has_value())
        << "returned optional SHOULD NOT have value";
  };
  "file_problems_v1"_test = [&] {
    std::ifstream ifstream_input_file("filetests.nonexistent_file",
                                      std::ios::in | std::ios::binary);
    auto vector_input_data =
        prepare_triviallyconstructiblestruct_vector_for_tests();
    auto recovered_optional =
        picklejar::read_vector_from_stream<TrivialStructure>(
            vector_input_data, ifstream_input_file);
    expect(false == recovered_optional.has_value())
        << "returned optional SHOULD NOT have value";
    auto recovered_optional_2 =
        picklejar::read_vector_from_file<TrivialStructure>(
            "filetests.nonexistent_file");
    expect(false == recovered_optional_2.has_value())
        << "returned optional SHOULD NOT have value";
  };

  "object_stream_v2"_test = [&] {
    const TestStructure test_object{};
    expect(true == picklejar::write_object_to_file(
                       test_object, "filetests.generated_test_data"));
    std::ifstream ifstream_input_file("filetests.generated_test_data",
                                      std::ios::in | std::ios::binary);
    auto recovered_optional = picklejar::read_object_from_stream<TestStructure>(
        ifstream_input_file,
        preserve_constructed_id_in_our_new_copy_and_modify_it);

    expect(true == recovered_optional.has_value())
        << "recovered optional must have a value";
    auto recovered_object{std::move(recovered_optional.value())};
    test_teststructure("object_stream_v2_", recovered_object, test_object,
                       startswith_modified_expected_modification);
  };

  "object_file_v2_problems"_test = [&] {
    std::ifstream ifstream_input_file("filetests.nonexistent_file",
                                      std::ios::in | std::ios::binary);
    auto recovered_optional = picklejar::read_object_from_stream<TestStructure>(
        ifstream_input_file, preserve_constructed_id_in_our_new_copy);
    expect(false == recovered_optional.has_value())
        << "returned optional SHOULD NOT have value";
    auto recovered_optional_2 = picklejar::read_object_from_file<TestStructure>(
        "filetests.nonexistent_file",
        preserve_constructed_id_in_our_new_copy_and_modify_it);
    expect(false == recovered_optional_2.has_value())
        << "returned optional SHOULD NOT have value";
    picklejar::ManagedAlignedStorageCopy<TestStructure> copy{};
    auto recovered_optional_3 = picklejar::read_object_from_file<TestStructure>(
        "filetests.nonexistent_file",
        preserve_constructed_id_in_our_new_copy_and_modify_it);
    expect(false == recovered_optional_3.has_value())
        << "returned optional SHOULD NOT have value";
  };
  "file_v2_problems"_test = [&] {
    std::ifstream ifstream_input_file("filetests.nonexistent_file",
                                      std::ios::in | std::ios::binary);
    auto vector_input_data = prepare_teststructure_vector_for_tests();
    auto recovered_optional = picklejar::read_vector_from_stream<TestStructure>(
        vector_input_data, ifstream_input_file,
        preserve_constructed_id_in_our_new_copy);
    expect(false == recovered_optional.has_value())
        << "returned optional SHOULD NOT have value";
    auto recovered_optional_2 = picklejar::read_vector_from_file<TestStructure>(
        "filetests.nonexistent_file", preserve_constructed_id_in_our_new_copy);
    expect(false == recovered_optional_2.has_value())
        << "returned optional SHOULD NOT have value";
  };
  "object_stream_v3_problems"_test = [&] {
    std::ifstream ifstream_input_file("filetests.nonexistent_file",
                                      std::ios::in | std::ios::binary);
    auto recovered_optional = picklejar::read_object_from_stream<TestStructure>(
        ifstream_input_file, preserve_constructed_id_in_our_new_copy,
        constructor_generator_one_param);
    expect(false == recovered_optional.has_value())
        << "returned optional SHOULD NOT have value";
  };
  "file_v3_problems"_test = [&] {
    auto recovered_optional = picklejar::read_vector_from_file<TestStructure>(
        "filetests.nonexistent_file", preserve_constructed_id_in_our_new_copy,
        constructor_generator_one_param);
    expect(false == recovered_optional.has_value())
        << "returned optional SHOULD NOT have value";
  };
  "stream_v3_problems"_test = [&] {
    std::ifstream ifstream_input_file("filetests.nonexistent_file",
                                      std::ios::in | std::ios::binary);
    std::vector<TestStructure> vector_input_data;
    auto recovered_optional = picklejar::read_vector_from_stream<TestStructure>(
        vector_input_data, ifstream_input_file,
        preserve_constructed_id_in_our_new_copy,
        constructor_generator_one_param);
    expect(false == recovered_optional.has_value())
        << "returned optional SHOULD NOT have value";
  };

  "object_stream_v2_problems"_test = [&] {
    std::ifstream ifstream_input_file("filetests.nonexistent_file",
                                      std::ios::in | std::ios::binary);
    auto recovered_optional = picklejar::read_object_from_stream<TestStructure>(
        ifstream_input_file, preserve_constructed_id_in_our_new_copy);
    expect(false == recovered_optional.has_value())
        << "returned optional SHOULD NOT have value";
  };
}
