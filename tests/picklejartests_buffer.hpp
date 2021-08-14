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

void picklejartests_buffer() {
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

    auto buffer_vector_copy_test = write_function(struct_vec);
    std::vector<TestStructure> buff_vec{};
    buff_vec.reserve(struct_vec.size());

    auto optional_read_vector =
        read_function(buff_vec, buffer_vector_copy_test);

    expect(true == optional_read_vector.has_value())
        << test_id << " "
        << "picklejar::read_vector_from_buffer<TestStructure>() failed";

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

  auto &&do_buffer_v1_test = [&](std::string test_id, auto &&write_function,
                                 auto &&read_function,
                                 const std::string &expected_modification,
                                 auto &&prepare_struct_vector_for_tests) {
    test_id += expected_modification;
    auto struct_vec = prepare_struct_vector_for_tests();

    auto buffer_vector_copy_test = write_function(struct_vec);
    std::vector<TrivialStructure> buff_vec{};
    buff_vec.reserve(struct_vec.size());

    auto optional_read_vector =
        read_function(buff_vec, buffer_vector_copy_test);

    expect(true == optional_read_vector.has_value())
        << test_id << " "
        << "picklejar::read_vector_from_buffer<TestStructure>() failed";

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

  auto &&buffer_write_function = [](auto &struct_vec) -> std::vector<char> {
    return {picklejar::write_vector_to_buffer(struct_vec)};
  };
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
  auto &&do_buffer_v1_test_w_data =
      [&](std::string test_id, auto &&write_function, auto test_data,
          auto &&prepare_struct_vector_for_tests) {
        do_buffer_v1_test(test_id, write_function, test_data.second,
                          test_data.first, prepare_struct_vector_for_tests);
      };

  // BUFFER_V3
  auto &&buffer_v3_firstconstructor_read_function =
      [&](auto &buff_vec, auto &buffer_vector_copy_test) {
        return picklejar::read_vector_from_buffer<TestStructure>(
            buff_vec, buffer_vector_copy_test,
            preserve_constructed_id_in_our_new_copy,
            constructor_generator_one_param);
      };

  auto &&buffer_v3_secondconstructor_read_function =
      [&](auto &buff_vec, auto &buffer_vector_copy_test) {
        return picklejar::read_vector_from_buffer<TestStructure>(
            buff_vec, buffer_vector_copy_test,
            preserve_constructed_id_in_our_new_copy,
            constructor_generator_two_params);
      };

  auto &&buffer_v3_modified_read_function = [&](auto &buff_vec,
                                                auto &buffer_vector_copy_test) {
    return picklejar::read_vector_from_buffer<TestStructure>(
        buff_vec, buffer_vector_copy_test,
        preserve_constructed_id_in_our_new_copy_and_modify_it,
        constructor_generator_two_params);
  };
  auto buffer_v3_firstconstructor_test_data =
      make_pair(startswith_firstconstructor_expected_modification,
                buffer_v3_firstconstructor_read_function);
  auto buffer_v3_secondconstructor_test_data =
      make_pair(startswith_secondconstructor_expected_modification,
                buffer_v3_secondconstructor_read_function);

  auto buffer_v3_modified_test_data =
      make_pair(startswith_modified_expected_modification,
                buffer_v3_modified_read_function);

  "buffer_v3"_test = [&] {
    do_buffer_test_w_data("buffer_v3_", buffer_write_function,
                          buffer_v3_firstconstructor_test_data,
                          prepare_teststructure_vector_for_tests);
    do_buffer_test_w_data("buffer_v3_", buffer_write_function,
                          buffer_v3_secondconstructor_test_data,
                          prepare_teststructure_vector_for_tests);
    do_buffer_test_w_data("buffer_v3_", buffer_write_function,
                          buffer_v3_modified_test_data,
                          prepare_teststructure_vector_for_tests);
  };

  // BUFFER_V2
  auto &&buffer_v2_notmodified_read_function =
      [&](auto &buff_vec, auto &buffer_vector_copy_test) {
        return picklejar::read_vector_from_buffer<TestStructure>(
            buff_vec, buffer_vector_copy_test,
            preserve_constructed_id_in_our_new_copy);
      };

  auto &&buffer_v2_modified_read_function = [&](auto &buff_vec,
                                                auto &buffer_vector_copy_test) {
    return picklejar::read_vector_from_buffer<TestStructure>(
        buff_vec, buffer_vector_copy_test,
        preserve_constructed_id_in_our_new_copy_and_modify_it);
  };
  const std::string startswith_default_expected_modification{"default"};
  auto buffer_v2_notmodified_test_data =
      make_pair(startswith_default_expected_modification,  // expect default
                buffer_v2_notmodified_read_function);

  auto buffer_v2_modified_test_data =
      make_pair(startswith_modified_expected_modification,
                buffer_v2_modified_read_function);

  "buffer_v2"_test = [&] {
    do_buffer_test_w_data("buffer_v2_", buffer_write_function,
                          buffer_v2_notmodified_test_data,
                          prepare_teststructure_vector_for_tests);
    do_buffer_test_w_data("buffer_v2_", buffer_write_function,
                          buffer_v2_modified_test_data,
                          prepare_teststructure_vector_for_tests);
  };

  // BUFFER_V1

  auto &&buffer_v1_read_function = [&]<class Type>(
                                       std::vector<Type> &buff_vec,
                                       auto &buffer_vector_copy_test) {
    return picklejar::read_vector_from_buffer<Type>(buff_vec,
                                                    buffer_vector_copy_test);
  };

  auto buffer_v1_test_data =
      make_pair(startswith_default_expected_modification,  // expect default
                buffer_v1_read_function);
  "buffer_v1_not_triviallycopiable_structs_test"_test = [&] {
    expect(false == std::is_trivially_copyable_v<TestStructure>)
        << "TestStructure SHOULD NOT be trivially copiable";
    expect(true == std::is_trivially_copyable_v<TrivialStructure>)
        << "This struct SHOULD be trivially copiable";
  };
  // THE FOLLOWING 2 SHOULD FAIL TO COMPILE BECAUSE THEY ARENT TRIVIALLY
  // COPIABLE
  /*"buffer_v1"_test = [&] {
    do_buffer_test_w_data("buffer_v1_", buffer_write_function,
                          buffer_v1_test_data,
    prepare_teststructure_vector_for_tests);
                          };*/
  /*"object_buffer_v1"_test = [&] {
    TestStructure test_object{"original"};
    picklejar::ManagedAlignedStorageCopy<TestStructure> copy{};
    auto test_buffer{picklejar::write_object_to_buffer(test_object)};
    size_t bytes_read_so_far{};
    auto recovered_object = *(read_object_from_buffer<TestStructure>(
                                  copy, test_buffer, bytes_read_so_far))
                                 .get_pointer_to_copy();
                                 };*/

  auto &&buffer_v1_read_function_inner = [&]<class Type>(
                                             std::vector<Type> &buff_vec,
                                             auto &buffer_vector_copy_test) {
    return picklejar::read_vector_from_buffer<Type>(buff_vec,
                                                    buffer_vector_copy_test);
  };

  auto buffer_v1_test_data_innerstruct =
      make_pair(startswith_default_expected_modification,  // expect default
                buffer_v1_read_function_inner);
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

  "buffer_v1_innerstruct"_test = [&] {
    do_buffer_v1_test_w_data(
        "buffer_v1_", buffer_write_function, buffer_v1_test_data_innerstruct,
        prepare_triviallyconstructiblestruct_vector_for_tests);
  };
  "object_buffer_v1_innerstruct"_test = [&] {
    TrivialStructure test_object{};
    auto test_buffer{picklejar::write_object_to_buffer(test_object)};
    size_t bytes_read_so_far{};
    auto recovered_object =
        picklejar::read_object_from_buffer<TrivialStructure>(test_buffer,
                                                             bytes_read_so_far);
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
  "object_buffer_v3"_test = [&] {
    const TestStructure test_object{};
    auto test_buffer{picklejar::write_object_to_buffer(test_object)};
    size_t bytes_read_so_far{};
    auto recovered_object = picklejar::read_object_from_buffer<TestStructure>(
        test_buffer, bytes_read_so_far, preserve_constructed_id_in_our_new_copy,
        constructor_generator_one_param);
    test_teststructure("buffer_v3_", recovered_object, test_object,
                       startswith_firstconstructor_expected_modification);
    bytes_read_so_far = 0;
    auto recovered_object_2 = picklejar::read_object_from_buffer<TestStructure>(
        test_buffer, bytes_read_so_far, preserve_constructed_id_in_our_new_copy,
        constructor_generator_two_params);
    test_teststructure("buffer_v3_", recovered_object_2, test_object,
                       startswith_secondconstructor_expected_modification);
    bytes_read_so_far = 0;
    auto recovered_object_3 = picklejar::read_object_from_buffer<TestStructure>(
        test_buffer, bytes_read_so_far,
        preserve_constructed_id_in_our_new_copy_and_modify_it,
        constructor_generator_two_params);
    test_teststructure("buffer_v3_", recovered_object_3, test_object,
                       startswith_modified_expected_modification);
  };

  "object_buffer_v2"_test = [&] {
    const TestStructure test_object{};
    auto test_buffer{picklejar::write_object_to_buffer(test_object)};
    size_t bytes_read_so_far{};
    TestStructure recovered_object =
        picklejar::read_object_from_buffer<TestStructure>(
            test_buffer, bytes_read_so_far,
            preserve_constructed_id_in_our_new_copy);
    test_teststructure("buffer_v2_", recovered_object, test_object,
                       startswith_default_expected_modification);
    bytes_read_so_far = 0;
    auto recovered_object_2 = picklejar::read_object_from_buffer<TestStructure>(
        test_buffer, bytes_read_so_far,
        preserve_constructed_id_in_our_new_copy_and_modify_it);
    test_teststructure("buffer_v2_", recovered_object_2, test_object,
                       startswith_modified_expected_modification);
  };

  // WRITE TO BUFFER ARRAY
  "write_object_to_buffer_array_version"_test = [&] {
    const TrivialStructure test_object{};
    auto test_buffer{picklejar::write_object_to_buffer_array(test_object)};
    size_t bytes_read_so_far{};
    auto recovered_object =
        picklejar::read_object_from_buffer<TrivialStructure>(test_buffer,
                                                             bytes_read_so_far);
    expect(true == (recovered_object == test_object))
        << "test object not equal to recovered object";
  };


  auto prepare_triviallyconstructiblestruct_array_for_tests =
    []() -> std::array<TrivialStructure, 4> {
    std::array<TrivialStructure, 4> struct_arr{};
    struct_arr.at(0).byte2_note_range_start = 1;
    struct_arr.at(0).byte2_note_range_end = 8738;
    struct_arr.at(1).byte2_note_range_start = 2;
    struct_arr.at(1).byte2_note_range_end = 8738;
    struct_arr.at(2).byte2_note_range_start = 3;
    struct_arr.at(2).byte2_note_range_end = 8738;
    struct_arr.at(3).byte2_note_range_start = 4;
    struct_arr.at(3).byte2_note_range_end = 8738;
    return struct_arr;
  };
  auto &&buffer_write_function_array = [](auto &struct_arr) -> std::array<char, 4*sizeof(TrivialStructure)> {
    return {picklejar::write_vector_to_buffer<TrivialStructure>(struct_arr)};
  };  
  "write_to_buffer_array_version"_test = [&] {
    do_buffer_v1_test_w_data(
        "buffer_v1_", buffer_write_function_array, buffer_v1_test_data_innerstruct,
        prepare_triviallyconstructiblestruct_array_for_tests);
  };
}
