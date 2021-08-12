#ifndef PICKLEJARTESTS_TESTSTRUCTURES_HPP // This is the include guard macro
#define PICKLEJARTESTS_TESTSTRUCTURES_HPP 1

#include <iostream>
#include <string>
#include <utility>
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

struct TrivialStructure {
  int byte2_note_range_start, byte2_note_range_end, byte3_item_current_idx,
      byte3_vel;
  enum class BYTE3 { any, lessthan, morethan };
  auto operator==(const TrivialStructure &comp) const -> bool {
    return byte2_note_range_start == comp.byte2_note_range_start &&
           byte2_note_range_end == comp.byte2_note_range_end &&
           byte3_item_current_idx == comp.byte3_item_current_idx &&
           byte3_vel == comp.byte3_vel;
  }
};

struct TestStructure {
  TrivialStructure note_range_selector;
  int status_byte_item_current_idx{}, transpose_n_notes{}, midi_channel = 1;
  std::string id{"default"};
  bool marked_for_deletion{false};
  bool marked_for_move{false};
  bool marked_move_direction_up{false};
  bool marked_move_direction_down{true};

  enum class STATUSBYTE { NotesRange, Control };
  auto operator==(const TestStructure &comp) const -> bool {
    return note_range_selector == comp.note_range_selector &&
           status_byte_item_current_idx == comp.status_byte_item_current_idx &&
           transpose_n_notes == comp.transpose_n_notes &&
           midi_channel == comp.midi_channel;
  }
  TestStructure() noexcept : note_range_selector{0, 127, 0, 63} {
    std::puts("TestStructure()");
  }
  auto operator=(const TestStructure &rhs) noexcept
      -> const TestStructure & = delete;
  auto operator=(TestStructure &&rhs) noexcept -> TestStructure & = delete;
  TestStructure(const TestStructure &rhs) noexcept
      : note_range_selector{rhs.note_range_selector} {
    std::puts(("copy(" + rhs.id + ")").c_str());
    id = rhs.id;

    status_byte_item_current_idx = rhs.status_byte_item_current_idx;
    transpose_n_notes = rhs.transpose_n_notes;
    midi_channel = rhs.midi_channel;
  }
  TestStructure(TestStructure &&rhs) noexcept
      : note_range_selector{std::exchange(rhs.note_range_selector, {})} {
    std::puts(("move(" + rhs.id + ")").c_str());
    id = std::exchange(rhs.id, {});
    status_byte_item_current_idx =
        std::exchange(rhs.status_byte_item_current_idx, {});
    transpose_n_notes = std::exchange(rhs.transpose_n_notes, {});
    midi_channel = std::exchange(rhs.midi_channel, {});
  }

  explicit TestStructure(std::string _id) noexcept
      : note_range_selector{0, 127, 0, 63},
        status_byte_item_current_idx{8738},
        id{std::move(_id)} {
    std::puts((std::string("TestStructure(") + id + ")").c_str());
  }
  explicit TestStructure(std::string /*_id*/, std::string _id2) noexcept
      : note_range_selector{0, 127, 0, 63},
        status_byte_item_current_idx{8738},
        id{std::move(_id2)} {
    std::puts((std::string("TestStructure(") + id + ")").c_str());
  }

  ~TestStructure() {
    std::puts((("~(") +
               // std::to_string(note_range_selector.byte2_note_range_start)
               id + ")")
                  .c_str());
    ;
  }
  void draw();
};
inline void print_vec(std::vector<TestStructure> &vector_data) {
  std::cout << "Reading contents of vector: \n";
  for (auto &val : vector_data) {
    std::cout << val.id << "= " << val.note_range_selector.byte2_note_range_end
              << ":" << val.midi_channel << ", ";
  }
  std::cout << "\nEND\n";
}

inline void print_vec(std::vector<TrivialStructure> &vector_data) {
  std::cout << "Reading contents of vector: \n";
  for (auto &val : vector_data) {
    std::cout << val.byte2_note_range_start << "= " << val.byte2_note_range_end
              << ":" << val.byte3_vel << ", ";
  }
  std::cout << "\nEND\n";
}
#endif
