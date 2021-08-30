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

#include <picklejar.hpp>

template <class IntBasedString>
static void step1_write_to_file(auto &intbased_vec) {
  if (picklejar::deep_copy_vector_to_file<1>(
          intbased_vec, "versioning_example.data",
          [](const IntBasedString &object) {
            return sizeof(IntBasedString::id) + object.rand_str_id.size();
          },
          [](auto &_ofs_output_file, const auto &object, size_t element_size) {
            auto total_size = size_t(_ofs_output_file.tellp());
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
    std::puts("WRITE_SUCCESS_STEP1");
  } else {
    std::puts("WRITE_ERROR_STEP1");
  }
}

template <class Container>
static auto step1_read_from_file(Container &read_result)
    -> picklejar::optional<Container> {
  if (auto optional_result{picklejar::deep_read_vector_from_file<1>(
          read_result, "versioning_example.data",
          [](auto &_result,
             picklejar::ByteVectorWithCounter &byte_vector_with_counter) {
            auto optional_id = byte_vector_with_counter.read<int>();
            if (!optional_id) return false;
            std::string _pretty_id(byte_vector_with_counter.current_iterator(),
                                   std::end(byte_vector_with_counter));
            // advance the byte counter by the remaning bytes
            if (!byte_vector_with_counter.advance_counter(
                    byte_vector_with_counter.size_remaining()))
              return false;

            _result.emplace_back(optional_id.value(), _pretty_id);
            return true;
          })}) {
    std::puts("READ_SUCCESS_STEP1");
    std::puts(
        ("fifth element_id=" + std::to_string(optional_result.value().at(4).id))
            .c_str());
    std::puts(
        ("fifth element_rand_id=" + optional_result.value().at(4).rand_str_id)
            .c_str());
    return PICKLEJAR_MAKE_OPTIONAL(optional_result.value());
  }
  std::puts("READ_ERROR_STEP1");
  return {};
}

static void step1() {
  // first we define the structure we want to store
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
  // then we generate 10 elements using our (int) constructor
  std::vector<IntBasedString> intbased_vec(10);
  std::generate(std::begin(intbased_vec), std::end(intbased_vec),
                [count = 0]() mutable { return IntBasedString{++count}; });

  // we call the write function to write the vector to the file
  step1_write_to_file<IntBasedString>(intbased_vec);
  std::vector<IntBasedString> read_result;
  // and lastly we read from the file.
  auto optional_read_result = step1_read_from_file(read_result);
  if (optional_read_result) {
    // do stuff with optional_read_result.value()
  }
}

template <class IntBasedString, class New_Pair, class Container>
auto step2_translate_v1_to_v2(Container &result_changed)
    -> picklejar::optional<Container> {
  if (auto optional_result{picklejar::deep_read_vector_from_file<1>(
          result_changed, "versioning_example.data",
          [new_elementgenerator = 0.](auto &_result,
                                      picklejar::ByteVectorWithCounter
                                          &byte_vector_with_counter) mutable {
            auto optional_id = byte_vector_with_counter.read<int>();
            if (!optional_id) return false;
            std::string _pretty_id(byte_vector_with_counter.current_iterator(),
                                   std::end(byte_vector_with_counter));
            // advance the byte counter by the remaning bytes
            if (!byte_vector_with_counter.advance_counter(
                    byte_vector_with_counter.size_remaining()))
              return false;

            // we added a new member so we need to generate it here
            std::vector<New_Pair> _new_important_pair_vector{
                {3., 1. * ++new_elementgenerator},
                {4., 1. * new_elementgenerator}};
            _result.emplace_back(optional_id.value(), _pretty_id,
                                 _new_important_pair_vector);
            return true;
          })}) {
    std::puts("READ_SUCCESS_STEP2_(V1_READ_TRANSLATE_TO_V2)");
    std::puts(("fifth element_id(V1_READ_TRANSLATE_TO_V2)=" +
               std::to_string(optional_result.value().at(4).id))
                  .c_str());
    std::puts(("fifth element_rand_id=(V1_READ_TRANSLATE_TO_V2)" +
               optional_result.value().at(4).rand_str_id)
                  .c_str());
    std::puts(("fifth element_new_important_pair_vector[1].second=" +
               std::to_string(optional_result.value()
                                  .at(4)
                                  .new_important_pair_vector.at(1)
                                  .second))
                  .c_str());
    return PICKLEJAR_MAKE_OPTIONAL(optional_result.value());
  }
  std::puts("READ_ERROR_STEP2_(V1_READ_TRANSLATE_TO_V2)");
  return {};
}

template <class IntBasedString, class New_Pair>
void step2_v2_write_function(auto &result_changed) {
  if (picklejar::deep_copy_vector_to_file<2>(
          result_changed, "versioning_example.data",
          [](const IntBasedString &object) {
            // we return the total size of elements we are writing into the file
            return /*our int id goes first*/
                picklejar::sizeof_unversioned(object.id) +
                /*then the size of our string*/
                picklejar::sizeof_unversioned(object.rand_str_id) +
                /*followed by the size of our vector of pairs*/
                picklejar::sizeof_unversioned(object.new_important_pair_vector);
          },
          [](auto &_ofs_output_file, const IntBasedString &object,
             size_t element_size) {
            // Any change in this lambda needs to be reflected on the other one,
            // if you update this one, you need to update the one that returns
            // the total size written as well write the id
            if (!picklejar::write_object_to_stream(object.id,
                                                   _ofs_output_file)) {
              return false;
            }

            /* the write_string_to_stream is equivalent to:
            // First) write the actual size of our string into the file
            if (!picklejar::write_object_to_stream(object.rand_str_id.size(),
                                                   _ofs_output_file)) {
              return false;
            }
            // Next) we write the string data into the file
            if (!picklejar::basic_stream_write(_ofs_output_file,
                                               object.rand_str_id.data(),
                                               object.rand_str_id.size()))
              return false;
            */
            // write the string into the file
            if (!picklejar::write_string_to_stream(object.rand_str_id,
                                                   _ofs_output_file)) {
              return false;
            }
            // and then we write the 'new_important_pair_vector' into the file
            if (!picklejar::write_vector_to_stream(
                    object.new_important_pair_vector, _ofs_output_file))
              return false;
            return true;
          })) {
    std::puts("WRITE_SUCCESS_V2");
  } else {
    std::puts("WRITE_ERROR_V2");
  }
}

template <class IntBasedString, class New_Pair, class Container>
auto step2_v2_read_function(Container &result_changed_v2)
    -> picklejar::optional<Container> {
  if (auto optional_result{picklejar::deep_read_vector_from_file<2>(
          result_changed_v2, "versioning_example.data",
          [](auto &_result,
             picklejar::ByteVectorWithCounter &byte_vector_with_counter) {
            auto optional_id = byte_vector_with_counter.read<int>();
            if (!optional_id) return false;
            auto optional_string_size = byte_vector_with_counter.read<size_t>();
            if (!optional_string_size) return false;
            std::string _pretty_id(byte_vector_with_counter.current_iterator(),
                                   byte_vector_with_counter.offset_iterator(
                                       optional_string_size.value()));
            if (!byte_vector_with_counter.advance_counter(
                    optional_string_size.value()))
              return false;

            // we added a new member so we need to generate it here
            std::vector<New_Pair> _new_important_pair_vector{};
            auto remaining_bytes =
                byte_vector_with_counter.get_remaining_bytes();
            auto optional_new_important_vector =
                picklejar::read_vector_from_buffer<New_Pair>(
                    _new_important_pair_vector, remaining_bytes,
                    [](auto &blank_instance,
                       auto &valid_bytes_from_new_blank_instance,
                       auto &bytes_from_file) {
                      picklejar::util::copy_new_bytes_to_instance(
                          bytes_from_file, blank_instance, sizeof(New_Pair));
                    });
            // advance the byte counter by the remaning bytes
            if (!byte_vector_with_counter.advance_counter(
                    byte_vector_with_counter.size_remaining()))
              return false;
            if (!optional_new_important_vector) return false;
            _result.emplace_back(optional_id.value(), _pretty_id,
                                 optional_new_important_vector.value());
            return true;
          })}) {
    std::puts("READ_SUCCESS_V2");
    std::puts(("fifth element_id(v2)=" +
               std::to_string(optional_result.value().at(4).id))
                  .c_str());
    std::puts(("fifth element_rand_id(v2)=" +
               optional_result.value().at(4).rand_str_id)
                  .c_str());

    std::puts(("fifth element_new_important_pair_vector[1].second=" +
               std::to_string(optional_result.value()
                                  .at(4)
                                  .new_important_pair_vector.at(1)
                                  .second))
                  .c_str());
    return PICKLEJAR_MAKE_OPTIONAL(optional_result.value());
  }
  std::puts("READ_ERROR_V2");
  return {};
}

void step2() {
  // now let's say we change our struct, we will have to use the old read
  // function to make it compatible with the new one
  using New_Pair = std::pair<double, double>;
  struct IntBasedString {
    std::string rand_str_id;
    int id;
    std::vector<New_Pair> new_important_pair_vector;
    IntBasedString() = default;
    explicit IntBasedString(int _id, const std::string _pretty_id,
                            std::vector<New_Pair> _new_important_pair_vector)
        : id(_id),
          rand_str_id(_pretty_id),
          new_important_pair_vector{_new_important_pair_vector} {
      std::puts((std::to_string(_id) + " with " + rand_str_id + " and with " +
                 std::to_string(new_important_pair_vector.size()) +
                 " new pairs Constructed")
                    .c_str());
    }
  };

  // Now we try to read using our v2 read version
  std::vector<IntBasedString> result_changed_v2;
  // after the next instruction if read is successful result_changed_v2 will be
  // moved inside the optional
  std::puts(
      "Attempting to read vector from file with 'step2_v2_read_function'");
  auto optional_result_changed_v2 =
      step2_v2_read_function<IntBasedString, New_Pair>(result_changed_v2);
  if (!optional_result_changed_v2) {
    // If our optional is empty that means that we failed to read the file and
    // it failed. Now we have to read using the version <1> but modified to use
    // our new 3 parameter constructor. Notice we are generating the
    // 'new_important_pair_vector' member which is why the lambda is mutable.
    // Notice we are reading into 'result_changed' variable
    std::puts(
        "Failed, Attempting to use 'step2_translate_v1_to_v2' as a Fallback");
    result_changed_v2.clear();  // we clear our vector first
    optional_result_changed_v2 =
        step2_translate_v1_to_v2<IntBasedString, New_Pair>(result_changed_v2);
    if (!optional_result_changed_v2) {
      std::puts("Error translating v1 to v2.");
      return;
    }
  }

  // And then write using the version <2> which needs to
  // be changed to add our 'new_important_pair_vector' member.
  // Notice we are using the 'result_changed' we just got from our previous read
  // operation
  step2_v2_write_function<IntBasedString, New_Pair>(
      optional_result_changed_v2.value());
  // After writing we have successfully updated our struct in the file and we
  // can now use the v2 versions to write and read from the file
}
void step3() {
  // we read with only the v2 version and write with the v2 version
  using New_Pair = std::pair<double, double>;
  struct IntBasedString {
    std::string rand_str_id;
    int id;
    std::vector<New_Pair> new_important_pair_vector;
    IntBasedString() = default;
    explicit IntBasedString(int _id, const std::string _pretty_id,
                            std::vector<New_Pair> _new_important_pair_vector)
        : id(_id),
          rand_str_id(_pretty_id),
          new_important_pair_vector{_new_important_pair_vector} {
      std::puts((std::to_string(_id) + " with " + rand_str_id + " and with " +
                 std::to_string(new_important_pair_vector.size()) +
                 " new pairs Constructed")
                    .c_str());
    }
    auto operator==(const IntBasedString &rhs) { return id == rhs.id; }
  };

  auto optional_version =
      picklejar::read_version_from_file("versioning_example.data");
  if (!optional_version) {
    std::puts("Failed to open file.");
    return;
  }
  if (optional_version.value() < 2) {
    std::puts(
        "Data file older than version 2 detected, this program only accepts "
        "data files version 2 or higher.");
    return;
  }

  std::vector<IntBasedString> result_changed_v2;
  // first we read from file
  auto optional_result_changed_v2 =
      step2_v2_read_function<IntBasedString, New_Pair>(result_changed_v2);
  if (!optional_result_changed_v2) {
    std::puts("Error reading v2 version from file.");
    return;
  }
  // do stuff with optional_result_changed_v2.value()

  // then we write from file on application end
  step2_v2_write_function<IntBasedString, New_Pair>(
      optional_result_changed_v2.value());
}

auto main(int argc, char *argv[]) -> int {
  if (argc <= 1) {
    std::puts(
        "\nPickleJar Versioning Example 2\nUsage: ./versioning_example stepN\n "
        "There are 4 steps meant to be be called one after the other that "
        "showcase the following example: \n step1) Assume you have written a "
        "program that uses the picklejar library to save/load a vector of "
        "'IntBasedString' objects into/from a file.\n step2) After releasing "
        "the program, you realize that you need to make some changes to "
        "'IntBasedString', your program now needs to accept 2 different "
        "versions of the file: v1 that was written in step 1, and a new "
        "version that takes the changes you have done in step2 into "
        "account.\n "
        "step3) Assume you have gone through this process a few times or some "
        "time has passed and you no longer want to support the version in "
        "step1 because everybody should have upgraded by now, in step3 you "
        "drop support of version1 by showing an error message if the version "
        "of the file is older than version 2.");
    return EXIT_FAILURE;
  }
  auto step = std::string(argv[1]);
  auto step_number = std::stoi(std::string(1, step.back()));
  std::puts("\n\nNEW APPLICATION RUN. STARTING...");
  std::puts(step.c_str());
  std::puts(("Step Number=" + std::to_string(step_number)).c_str());
  switch (step_number) {
    case 1:
      step1();
      break;
    case 2:
      step2();
      break;
    case 3:
      step3();
      break;
  }
}
