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
#include "picklejartests_buffer.hpp"
#include "picklejartests_file.hpp"

using namespace boost::ut;

int main() {
  picklejartests_file();
  picklejartests_buffer();
  // namespace u = boost::ut;
  // using namespace boost::ut::literals;
  // using namespace boost::ut::operators::terse;
  "Int Vector"_test = [] {
    std::vector<int> int_vec{0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

    expect(true == picklejar::write_vector_to_file(int_vec, "example1.data"))
        << "picklejar::write_vector_to_file(int_vec...) - Write Failed";
    auto optional_read_vector =
        picklejar::read_vector_from_file<int>("example1.data");

    expect(true == optional_read_vector.has_value())
        << "picklejar::read_vector_from_file<int>() failed";
    expect(true == std::equal(std::begin(optional_read_vector.value()),
                              std::end(optional_read_vector.value()),
                              std::begin(int_vec)))
        << "Read vector is not equal to vector used to test";
    if (optional_read_vector) {
      print_vec(optional_read_vector.value());
    }
  };
  "String Vector"_test = [] {
    std::vector<std::string> string_vec{"",   "1",  "2",   "4",   "8",  "16",
                                        "32", "64", "128", "256", "512"};

    expect(true == picklejar::write_vector_to_file(string_vec, "example1.data"))
        << "picklejar::write_vector_to_file(int_string...) - Write Failed";
    auto optional_read_vector = picklejar::read_vector_from_file<std::string>(
        "example1.data", [count = 1](auto &blank_instance,
                                     auto &valid_bytes_from_new_blank_instance,
                                     auto &bytes_from_file) mutable {
          picklejar::util::preserve_blank_instance_member(
              0, sizeof(std::string), valid_bytes_from_new_blank_instance,
              bytes_from_file);
          picklejar::util::copy_new_bytes_to_instance(
              valid_bytes_from_new_blank_instance, blank_instance,
              sizeof(std::string));
          blank_instance = "prefix" + std::to_string(++count);
        });

    expect(true == optional_read_vector.has_value())
        << "picklejar::read_vector_from_file<std::string>() failed";
    // this should be false
    expect(false == std::equal(std::begin(optional_read_vector.value()),
                               std::end(optional_read_vector.value()),
                               std::begin(string_vec)))
        << "Read vector is equal to vector used to test and it should have "
           "been modified";

    // this should be false
    expect(true ==
           std::all_of(std::begin(optional_read_vector.value()),
                       std::end(optional_read_vector.value()),
                       [](const auto &s) { return s.starts_with("prefix"); }))
        << "Read vector is should contain elements starting with 'prefix'";
    if (optional_read_vector.has_value()) {
      print_vec(optional_read_vector.value());
    }
  };
}
