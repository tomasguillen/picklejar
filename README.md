### Apache License:
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

### Feel free to contact me
Send an email to tomguillen at zoho dot com, if you find a bug or have any comments or ideas you would like to see added to the library. Any code contribution will be welcomed and considered.

# C++ PickleJar
Save and Load Objects and Vectors and Arrays from/to files, ifstreams or byte buffers; a simple versioning system prevents you from making common mistakes, and it allows you to update the objects stored after the fact.

## The Ultimate Goal Of This Library
In an ideal world, you could just call ```picklejar::pickle_write_or_read(any_kind_of_object);``` and it would do the right thing. Unfortunately, c++20 doesn't have a way to reflect on the value member types of a class, so I can't loop through each member and use the right API call in the right circumstance. Maybe reflection will appear in c++23, but until then you will have to use one of the two APIs I've provided below.

## Two APIs:
* One for deep copying/reading with versioning and byte size redundancy
* And a different lower level API that just writes the bytes of the object; and has a number of read functions to account for simple and complex object caveats.

## Highlights:
1. Both APIs work with streams, files and arbitrary buffers of bytes.
2. You can mix both APIs to have redundancy and speed where needed: *deep_copy_*, and *deep_read_*, for reliability. And *write_*, and *read_*, for faster low level operations. The latter should only be used for objects that have sequential storage; unless, you plan to ignore non-sequential items. For example, a map doesn't have sequential storage so you would use the *deep_\** API; but, if you have an object that contains a map, and you want to ignore the map, and read the other value members; the read API has a way to do that. See [Solution 3 and 4](https://github.com/tomasguillen/picklejar#solution-3-re-generate-the-object-using-its-constructor).
3. Uses c++20 concepts in order to catch common errors at compile-time, that would otherwise cause a program to crash at run-time.

## Adding PickleJar to your project
### 1. CMake 
```git
git clone --recursive https://github.com/tomasguillen/picklejar.git
```
Modify your CMakeLists.txt:
```cmake
add_subdirectory(picklejar)

add_executable(target_name ...
target_link_libraries(target_name PRIVATE PickleJar ...
```
and in your code add:
```c++
#include <picklejar.hpp>
...
```
### 2. Without CMake
```git
git clone --recursive https://github.com/tomasguillen/picklejar.git
```
then in your code add:
```c++
#define DISABLE_TYPESAFE_OPTIONAL
#include "picklejar/include/picklejar.hpp"
...
```

## Quick Start
If you are trying to store the following types—or structs and classes that contain only the following types:
int, double, float, bool, std::array or a normal array but stored inside the object, and any other type where it's bytes are not stored in the heap or outside of the object allocated memory. Then you can, and should prefer to use the low-level API, but only if you don't need versioning.

If you are trying to store an object allocated in the heap, or a hybrid like a std::string, you should use the deep_(copy/read) API. With the exception of std::string that now has it's own **write_string_to_(stream/file/buffer)** function, but no read function since it's a simple case.

Every read or write function for both APIs has a version that interfaces with the following 3 types: (std::ofstream or std::ifstream), a file name (just a wrapper around the stream version), and a picklejar::BufferVectorWithCounter. In the case of the BufferVectorWithCounter class it's just essentially a **std::vector<char>** that has a counter that increases when it's .read() or .write() member functions are called. Their functionality is very similar, their use only differs slightly, and in some cases there's no difference at all.

Similarly, every read or write function has an object and a vector version. The latter can work with any container but only for it's **deep_(copy/read)** version.

### Low-Level API Quick Start
All low-level read or write functions have the following form:\
**picklejar::write_object_to_file**, where you can replace "write" with "read", "object" with "vector", and "file" with one of: "stream", "file", "buffer".\
Additionally, this API has **picklejar::util::preserve_blank_instance_member** and **copy_new_bytes_to_instance** which are described in the Low-Level API break down section.\
Also, **picklejar::sizeof_unversioned** can be used to obtain the size of an object or vector taking into account additional bytes used by picklejar—generally just the .size() if it's a container or a string. This is useful for using low-level API calls inside the Deep Copy/Read API write functions.\
Finally, **picklejar::write_string_to_buffer** is a lone function, there is no read equivalent and it's just there to facilitate writting std::strings in certain situations, reading the string is easy and can be seen in the versioining examples.

Here are the write and read functions that can be used to write and read an object to a file
```c++
picklejar::write_object_to_file(5, "example1.data"); // returns true if successful
int i = picklejar::read_object_from_file<int>("example1.data"); // returns the int, notice we have to pass the type as a template paramater <int>.
```
Here is how to write and read a vector of ints to/from a file:
```c++
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
```
The Low-Level API won't easily work with a vector of std::string, unless you use it's more complex overloads, I recommend using the Deep Copy/Read API explained next. See the "Low-Level API Break Down Section" of this readme to know more it.

### Deep Copy/Read API
All deep copy or deep read functions have the following form:\
**picklejar::deep_copy_object_to_file**, where you can replace "copy" with "read", "object" with "vector", and "file" with one of: "stream", "file", "buffer".\
Finally, **picklejar::sizeof_versioned** can be used to obtain the size of a **deep_copied** object or vector taking into account additional bytes used by picklejar—generally consisting of the version and the .size() if it's a container or a string. This is useful for using nested **deep_copy** API calls inside another Deep Copy/Read API write function.

Here is how to write and read a vector of std::string:
```c++
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
}
```
### Basic API Quick Start
All Basic API read or write functions have the following form:\
**basic_stream_write**, where you can replace "write" with "read", and "stream" with "buffer". There is no "file" version.

They all take 3 paramaters:
1. a stream(ofstream or ifstream) or a picklejar::ByteVectorWithCounter.
2. a pointer to **destination_to_read_from** in case of the write function. and a pointer to **destination_to_write_to** in case of the read function. See example after.
3. The size to read or write.

```c++
// write string starting with .data() pointer until .data() + size effectively writting the whole string to the std::ofstream
picklejar::basic_stream_write(_ofs_output_file, a_std_string.data(), a_std_string.size());
```
For the read function we are telling it to read from the stream or buffer into the second parameter pointer we passed.

### About ByteVectorWithCounter
Here is the basic functionality—just a vector of char with a counter:
```c++
ByteVectorWithCounter byte_vector_with_counter{sizeof(int)}; // tell it how much space you need
byte_vector_with_counter.write(5); // returns true if successful write, it will deduce the type we pass it
byte_vector_with_counter.set_counter(0); // resets the counter so we can read what we just wrote
int i = byte_vector_with_counter.read<int>(); // reads the int into variable i, it can't deduce so we have to pass <int> as a template parameter

```
you can directly access **.byte_data** which is the **vector<char>** and also **byte_counter** which is a **std::optional<size_t>** that is invalidated if we try to read or write more than it's current size.

# Deep Copy/Read API break down section
## Versioning System
Only the deep copy/read API is setup to be able to write versioned objects and vectors, you can see a complete example that uses all the capabilites of this library in *examples/versioning_example.cpp* and *examples/versioning_example_2.cpp*, the second is a copy of the first with one more step and they have very similar usage.

If you are trying to store non trivial types, I highly recommend you compile the versioning examples; since they pretty much show the whole point of this library. You may want to modify the code, to store and write the structures you are interested in; just look at how it works for the IntBasedString struct in the *versioning_example* files. A unit test is setup here: *tests/versioning_example_2_with_tests.cpp*; you can use it as a template for the structures you are interested in. It uses a thirdparty unit test library which is located in the *tests/ut* directory. It should be easy to change it to a different test framework. It's always nice to have tests setup for this type of thing because it allows to detect any changes that may need to be fixed in case anything outside your control changes. Ex: standard or compiler changes that can't be anticipated.

##### The versioning examples are located under the *examples* directory and work like this:
Run the program from the command line: *./versioning_example step1*; there are a total of 3 steps for the: *./versioning_example* executable; and 4 steps for: *./versioning_example_2*. The only difference between these two is step4 has been added.

* step1) Assume you have written a program that uses the picklejar library to save and load a vector of 'IntBasedString' objects into/from a file.

* step2) After releasing the program; you realize that you need to make some changes to 'IntBasedString'. Your program now needs to accept 2 different versions of the file: *version 1* that was written in step1; and a new version that takes the changes you have done in step2 into account. For this you need 3 different functions:
1. One that is able to read the version 1 file and translate it into your structure taking into account the changes made.
2. A new write function with the version number bumped up and taking new changes into account.
3. A new read function with the version number bumped up and taking new changes into account.

* step3) Assume you've gone through this process a few times; or, maybe some time has passed, and you no longer want to support the version in step1 because everybody should have upgraded by now. So in step3 you drop support of *version 1* by showing an error message if the version of the file is older than *version 2*.

* step4) Similar to step2; we change our IntBasedString to contain a *map<string,trivial_object>*. Once again we have to accept 2 different versions: *version 2*, and our new *version 4*. Something to note: Map requires using deep copy because it's not sequential. So we've added a versioned *deep_copy* call for our map inside our *deep_copy* for the IntBasedString struct.

##### Here are some intended effects of writting the previous example using the PickleJar library:

If you run step1, followed by step3 the program will output the following message:
``Data file older than version 2 detected, this program only accepts data files version 2 or higher.``

If you run step1, then step2. The program will first try to read the file with it's *version2 read function*, it will fail, and then fallback to it's *version1 to version2 conversion function* and succeed:
```
Attempting to read vector from file with 'step2_v2_read_function'
PICKLEJAR_VERBOSE_MODE: Non-critical condition: `optional_version.value() == Version` failed in /mnt/1TUnifiedExtra/lnLinkedSystemFilesAndSoundLibrariesForMusicProduction/ln_home_tom_builds/benchmark/picklejar/examples/picklejar/include/picklejar.hpp line 1593: PICKLEJAR_RUNTIME_MESSAGE: The version from the file (1) doesn't match with the Version of the function (2)
READ_ERROR_V2
Failed, Attempting to use 'step2_translate_v1_to_v2' as a Fallback
```

By default PickleJar is configured in verbose mode and it will output a message to std::cerr, as you can see the version of the file is *version 1* and the version of the *step2 read function* is version 2. So it will fail to read it. You can disable verbose mode by setting **PICKLEJAR_ENABLE_VERBOSE_MODE** macro to 0 before you include the file or from the command line or with cmake:
```cmake
target_compile_definitions(cmake_target_name PRIVATE PICKLEJAR_ENABLE_VERBOSE_MODE=0)
```
Only non-critical warnings will be disabled by this setting.

If you run step1, followed by step2, and then by step2 again, in the last run, the program will no longer have to convert from version 1 to version 2 and you won't see that warning message:
```
Attempting to read vector from file with 'step2_v2_read_function'
... Constructor Output ...
READ_SUCCESS_V2
```

If you run step1, followed by step2, and then by step3 it will read everything just fine, it will only show an error if you run step1 and then step3 because the version won't match as seen previously.

If you run step1, followed by step2, and then by step4 (step3 doesn't matter here), you will get behavior similar to what happens with step2 but with the version 2 being translated to version 4. If you then try to run step2 you will get 2 warnings, one for the *version 2 read function* and another for the *version 1 to version 2 conversion function* which is just what was intended.

###### The versioning unit tests are located in *tests/versioning_example_2_with_tests.cpp*
It runs all the steps in the order they are meant to be tested. It can be used as a template to create a unit tests for your own structures that you want to save and load to a file. I recommend to add a new test every time you make lasting changes to the saved structs you are planning to release.

## Break down of the versioning example code
The code mixes the low level **(write/read)_** API and the **deep_(copy/read)** API. Let's go through the step1 code, first we setup a vector that contains the elements we want to store and we call a write and read function that we will break down next:
```c++
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
```
#### Break down of the write function.
```c++
template <class IntBasedString>
static void step1_write_to_file(auto &intbased_vec) {
  if (picklejar::deep_copy_vector_to_file<1>(
          intbased_vec, "versioning_example.data",
          [](const IntBasedString &object) {
            return sizeof(IntBasedString::id) + object.rand_str_id.size();
          },
          [](auto &_ofs_output_file, const auto &object, size_t element_size) {
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
```
I think it would be better if this function would return true if successful, but since it's only an example it prints "WRITE\_SUCCESS\_STEP1" if successful.

deep\_copy\_vector\_to\_file template parameter:\
Now, let's focus on the call to `picklejar::deep_copy_vector_to_file<1>(`, the first thing you should notice is the \<1\> template parameter; this is meant to be the version of the object——And is what allows us to read the file version in step3, drop support for older versions, and be able to fallback to a more suitable read function version in step2. If you change it to \<0\>, it would disable versioning, basically you would save sizeof(size_t) which is 8 bytes in my system; but in exchange you lose the mentioned functionality. It's handy in some situations——like structures and classes that you know will never change and don't need the versioning information——and it's used for internal calls by PickleJar when it's not needed.

Now for the deep\_copy\_vector\_to\_file function parameters:
1. We first pass the vector from which PickleJar will read the objects and write them into the file.
2. The second parameter is the file name that will be used to write the bytes into.
3. The third parameter is a lambda that takes each object as a parameter and returns the size of each value member. This has to be a lambda because some objects can have variable size; since we have a string inside IntBasedString struct, we want to add the number of characters of each string to the sizeof int id member—which so far, are the only value members of the structure.
4. The fourth parameter is another lambda, this lambda takes: a std::ofstream& which is what we are going to use to write every element we want to preserve into the file. This lambda is called once for each object inside the vector, so the lambda's second parameter is one of the objects inside the vector. The third parameter is the total size to be written, which is the same as what we return in the previous third parameter lambda.

Let's look at the fourth parameter lambda in more detail:
```c++
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
```
This uses the low level API, to write the data of each member into the file. ```picklejar::write_object_to_stream(object.id, _ofs_output_file)``` writes the bytes of our int, you can replace **object.id** for any simple type, even simple structs. For example, structs that only contain: ints, chars, doubles, arrays of simple types, other structs of simple types will all work with this low level call.

So, we have written a simple int, now we use ```picklejar::basic_stream_write(_ofs_output_file, object.rand_str_id.data(), object.rand_str_id.size())``` to write each character of our string into the file. **basic_stream_write** returns true if successful. Notice how we haven't stored how big our string is, we can get away with this only because it's the last element we are writting and PickleJar already knows the total size of bytes of the object—which we returned it in the third parameter lambda:
```c++
return sizeof(IntBasedString::id) + object.rand_str_id.size();
```
However, if you change your struct and you want to add another member that has variable size, we can't get away with this anymore and we will see how to deal with that in step2.

#### Break down of the read function
First thing to note is the next function returns a **picklejar::optional<Container>**, which if successful will have as a .value() the actual container that we passed into this function by reference: **(Container &read_result)**. PickleJar by default works with a thirdparty typesafe library. I wanted to have a way to return an optional of a container—technically speaking, you can return a pointer that is null when the result is invalid—but I wanted to use an optional because it looks nicer and more expressive in my opinion. The problem is if I return a **std**::optional\<Container\> there will be a copy or a move into the container. Because this could be a container with a lot of elements; I didn't want to copy and I disliked having a move too even though it's not that bad. Another way would be to return an optional of a pointer to the container, but I disliked the pointer interface and I already had existing code.

So I opted to add a thirdparty library and hide it under **picklejar::optional<Container>** what this means to you is that by default PickleJar returns an optional that contains a reference—in simple words it's a class that contains a pointer that behaves like a reference—That way, you can use it as if it's a std::optional<Container> but without a move or a copy. You can disable the thirdparty library by setting **DISABLE_TYPESAFE_OPTIONAL** macro to 1 before you include the file or from the command line or with cmake:
```cmake
target_compile_definitions(cmake_target_name PRIVATE DISABLE_TYPESAFE_OPTIONAL=1)
```

If you choose to use thirdparty *optional_ref* or *std::optional* and you don't want to return a *picklejar::optional*, take a look at the top of "picklejar.hpp" and copy either method or use your own prefered way for your functions. If you use picklejar::optional and later disable or enable the thirdparty library you won't have to change your code as long as you are checking the result and using the .value() inside the if statement. Like in the following *read function*:
```c++
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
```

```picklejar::deep_read_vector_from_file<1>``` will return a picklejar::optional, which if successful we use it's stored value like **optional_result.value()**, this is the vector that was read from the file.

The template parameter is the version of the file this read function is able to read, anything else will be rejected. Same as explained in the write function, \<0\> will disable versioning.

deep\_read\_vector\_to\_file parameters:
1. We first pass the vector into which PickleJar will insert the objects it reads from the file. If the read is successful it will return an optional that contains a reference to that vector and is accessible by optional_result.value().
2. The second parameter is the file name to read from
3. The last parameter is a lambda that takes care of translating the bytes in the file into an element that will be inserted into the vector. Returns true if read is successful. Basically, it's the counterpart for the fourth parameter of the *deep_copy_vector_to_file* function.

Let's look at the lambda in more detail:
```c++
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
          }
```
It needs to take two parameters: a reference to the same vector that we passed as a first parameter to *deep_read_vector_to_file*, and a **picklejar::ByteVectorWithCounter** which is a vector<char> that has a counter that keeps track of how many bytes have been read so far. This second parameter contains the bytes we got from the file that we need to convert into a valid object.

```c++
auto optional_id = byte_vector_with_counter.read<int>();
```
What this line does is read 4 bytes—sizeof(int)—from our buffer and return an **std::optional** that contains our id member. So we have effectively recovered our stored IntBasedString::id member from the file. It also takes care of advancing the inner counter of *byte_vector_with_counter* by 4 bytes. Setting up our call to byte_vector_with_counter.current_iterator() for the subsequent *std::string* read...
```c++
            std::string _pretty_id(byte_vector_with_counter.current_iterator(),
                                   std::end(byte_vector_with_counter));
```

To recover our *std::string*, we simply pass two iterators—representing the remaining bytes of the buffer—to the string constructor. We also have to advance the counter of our *byte_vector_with_counter* to the end; otherwise, PickleJar will show a run time error—it's an assert— telling you that you haven't used all the bytes that were expected:
```
byte_vector_with_counter.advance_counter(byte_vector_with_counter.size_remaining())
```

Finally, we construct the object inplace with std::vector::emplace_back by passing our recovered members. You can use *push\_back* if you want but emplace_back should be the correct way here, you can also pass things other than a vector to this function; so technically, you can insert into a list or a map if you wish.
```
_result.emplace_back(optional_id.value(), _pretty_id);
```
Remember to return true if successful.

### Now that we know the basics of how the deep\_(copy/read) API works let's quickly go over the other steps in the versioning example:
#### Step2:
In step2 we need three functions instead of two:
##### step2\_translate\_v1\_to\_v2:
Reads the file from version one and adds a new element we can pass to our constructor. Alternatively, you can use the same read_function you used in version 1 but change the 2 parameter constructor to deal with the changes made to the struct.
##### step2\_v2\_write\_function:
Here we write the int that contains our id member, like we did in step1; but we changed our string to use:
```c++
picklejar::write_string_to_stream(object.rand_str_id, _ofs_output_file)
```
This function will store size information additionaly to the string's character data so we can know how many bytes to read later for each object. This is done because each object can contain a differently sized string.
Then we store our vector of pairs:
```c++
            if (!picklejar::write_vector_to_stream(
                    object.new_important_pair_vector, _ofs_output_file))
              return false;
```
This will just take the vector's .data() and write it to the file. This works because vector storage is sequential. And finally, since it's the last item we are storing, we don't need to store the size of the vector.

Another thing to note is that since now we are using low level API calls we can obtain the size of each object using **picklejar::sizeof_unversioned(object)**. We do this in the third parameter lambda that takes care of returning the size we are writing:
```c++
          [](const IntBasedString &object) {
            // we return the total size of elements we are writing into the file
            return /*our int id goes first*/
                picklejar::sizeof_unversioned(object.id) +
                /*then the size of our string*/
                picklejar::sizeof_unversioned(object.rand_str_id) +
                /*followed by the size of our vector of pairs*/
                picklejar::sizeof_unversioned(object.new_important_pair_vector);
          }
```
##### step2\_v2\_read\_function:
Here we read the int as we did before, but it changes for the string, we first obtain the string size:
```c++
auto optional_string_size = byte_vector_with_counter.read<size_t>();
```
Then we use it to initialize our string
```c++
            std::string _pretty_id(byte_vector_with_counter.current_iterator(),
                                   byte_vector_with_counter.offset_iterator(
                                       optional_string_size.value()));
```
**byte_vector_with_counter.offset_iterator** returns an iterator that is equivalent to **current_iterator()** + **optional_string_size.value()**
We still have to advance the counter by the size of the string.

Then we proceed to read the new vector of pairs from the remaing bytes in the **byte_vector_with_counter** using the low level API—which is explained in another section of this readme, but basically, it just copies the bytes from the file directly into a blank instance of **New_Pair**—passing it the **byte_vector_with_counter**:
```c++
            std::vector<New_Pair> _new_important_pair_vector{};
			auto optional_new_important_vector =
                picklejar::read_vector_from_buffer<New_Pair>(
                    _new_important_pair_vector, byte_vector_with_counter,
                    [](auto &blank_instance,
                       auto &valid_bytes_from_new_blank_instance,
                       auto &bytes_from_file) {
                      picklejar::util::copy_new_bytes_to_instance(
                          bytes_from_file, blank_instance, sizeof(New_Pair));
                    });
```
Notice we don't have to advance the counter when we use a _picklejar::read_(object/vector)__from_**buffer** function. Any function that takes a **byte_vector_with_counter** will take care of advancing the counter for us.

We end by emplacing the object with it's additional third parameter:
```c++
_result.emplace_back(optional_id.value(), _pretty_id,
                                 optional_new_important_vector.value());
return true;
```
#### Step3:
Step3 reuses the step2 read and write functions, instead of translating from version one we drop support like so:
```c++
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
```

#### Step4:
Is very similar to step2 in that we have copied the three functions used and modified them to add a map to our IntBasedString example structure.
##### step4\_translate\_v2\_to\_v4:
Only differs from step2 in that we pass a generated map element to the emplace_back call.
##### step4\_v4\_write\_function:
We added a **deep_copy** API call to store our map. One important detail is that we are storing the map before our vector of pairs, in order to take advantage of the low level API call that we already wrote in step2. Otherwise we would have to use **deep_copy** for the vector of pairs too since we would have to know it's size ahead of time.
```c++
            if (!picklejar::deep_copy_vector_to_stream<1>(
                    object.new_map, _ofs_output_file,
                    [](auto &map_elem) {
                      return picklejar::sizeof_unversioned(map_elem.first) +
                             picklejar::sizeof_unversioned(map_elem.second);
                    },
                    [](auto &_map_ofs_output_file, auto &map_elem,
                       size_t map_element_size) {
                      // write the string into the file
                      if (!picklejar::write_string_to_stream(
                              map_elem.first, _map_ofs_output_file)) {
                        return false;
                      }
                      // next we write the map value
                      if (!picklejar::write_object_to_stream(
                              map_elem.second, _map_ofs_output_file)) {
                        return false;
                      }
                      return true;
                    })) {
              return false;
            }
```
Another important point is that now that we are using a **deep_copy** API we now use **picklejar::sizeof_versioned<1>** with the same version as the map **deep_copy** call:
```c++
          [](const IntBasedString &object) {
            // we return the total size of elements we are writing into the file
            return /*our int id goes first*/
                picklejar::sizeof_unversioned(object.id) +
                /*then the size of our string*/
                picklejar::sizeof_unversioned(object.rand_str_id) +
                /*followed by our new_map size*/
                picklejar::sizeof_versioned<1>(object.new_map) +
                /*followed by the size of our vector of pairs*/
                picklejar::sizeof_unversioned(object.new_important_pair_vector);
          }
```
##### step4\_v4\_read\_function:
The only change from step2 here is that we read the map, before we read our vector of new pairs:
```c++
            New_Map read_new_map{};
            auto optional_new_map{picklejar::deep_read_vector_from_buffer<1>(
                read_new_map, byte_vector_with_counter,
                [](auto &map_result, picklejar::ByteVectorWithCounter
                                         &_map_byte_vector_with_counter) {
                  auto optional_map_key_size =
                      _map_byte_vector_with_counter.read<size_t>();
                  if (!optional_map_key_size) return false;
                  std::string _map_key(
                      _map_byte_vector_with_counter.current_iterator(),
                      _map_byte_vector_with_counter.offset_iterator(
                          optional_map_key_size.value()));
                  if (!_map_byte_vector_with_counter.advance_counter(
                          optional_map_key_size.value()))
                    return false;

                  auto optional_map_value =
                      _map_byte_vector_with_counter.read<TrivialStructForMap>();
                  if (!optional_map_value) return false;
                  map_result[_map_key] = optional_map_value.value();
                  return true;
                })};
```
first we read our std::string key:
```c++
                auto optional_map_key_size =
                      _map_byte_vector_with_counter.read<size_t>();
                  if (!optional_map_key_size) return false;
                  std::string _map_key(
                      _map_byte_vector_with_counter.current_iterator(),
                      _map_byte_vector_with_counter.offset_iterator(
                          optional_map_key_size.value()));
                  if (!_map_byte_vector_with_counter.advance_counter(
                          optional_map_key_size.value()))
                    return false;
```
then we read the value:
```c++
auto optional_map_value =
                      _map_byte_vector_with_counter.read<TrivialStructForMap>();
                  if (!optional_map_value) return false;
```
and finally we insert our read elements into the resulting map:
```c++
map_result[_map_key] = optional_map_value.value();
```

# Low-Level API break down section
## How to use PickleJar to store and recall Trivial Types (ints, floats, doubles, simple structs and classes, etc)
This is how to write a vector of ints to a file named "example1.data":
```c++
  std::vector<int> int_vec{0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

  if (picklejar::write_vector_to_file(int_vec, "example1.data"))
    std::puts("WRITESUCCESS");
```
```write_vector_to_file``` returns true if writting to the file was successful.

And this is how we can read it from that same file:
```c++
  if (auto optional_read_vector =
          picklejar::read_vector_from_file<int>("example1.data");
      optional_read_vector) {
    std::puts(("READSUCCESS: fifth element is " +
               std::to_string(optional_read_vector.value().at(4)))
                  .c_str());
  }
```
```read_vector_from_file<int>``` will return an object of type ```std::optional<std::vector>```. The if statement checks if there was a problem reading the file and to get our new vector we use ```optional_read_vector.value()```
Putting these two together we get the following output:
```
WRITESUCCESS
READSUCCESS: fifth element is 8
```
Notice how we passed ```<int>``` as a template parameter to the read function, this is because we stored ints in the **write_vector_to_file** operation.

## What can we do for Non-TriviallyCopiable Types
There are 4 things you can do for Non-TriviallyCopiable types:
1. Solution 1: Copy the size of the non-triviallycopiable object along with it's bytes (this way we know how many characters to read/store)
2. Solution 2: Ignore it and Re-Generate the Non-TriviallyCopiable object when we run the program again.
3. Solution 3: Re-Generate the object using it's constructor (similar to solution 2 but constructed in place).
4. Solution 4: Based in **Solution 3**. Ignore the value and instead use it's default constructor (each solution may be useful on certain situations).
## Solution 1: Copy the size of the non-triviallycopiable object along with it's bytes (this way we know how many characters to read/store)
In order to be able to "deep copy" our string, we will need to store how many bytes each string has just before we store the data, this can be done with PickleJar like so (using **deep_copy_vector_to_file** and **deep_read_vector_from_file**):
```c++
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
}
```
* There are 2 lambdas for the write function:
1. The first lambda takes an element of the vector to be stored as a parameter and is responsible of returning the number of bytes we need to store. In the case of a string it takes a **std::string** and returns the **std::string::size()**.
2. The second lambda takes the ofstream, an object(in this case a std::string) and the size that is returned by the first lambda. This function is responsible of calling the write function for ofstream with the characters of our std::string and the length to copy into the file. It must return true if the write is successful which is why we return ```_ofs_output_file.good()```.

This allows to use this function for more complex types and all you have to do is tell it how to get the size of the bytes of the object you want to store and where the bytes are.

* There is one lambda for the read function, this lambda is in charge of taking a vector of bytes, that contains our string characters in this case, and constructing our string in place into our result vector.
## Solution 1: For a more complex case.

## Solution 2: Don't save it and Re-Generate the Non-TriviallyCopiable object when we run the program again.
This Solution works if you can generate the object without much effort and the non-triviallycopiable object has a default constructor(otherwise see Solution 3 which is prefered to Solution 2), in other words you don't need an exact copy because the data can be generated (using **write_vector_to_file** and **read_vector_from_file\<std::string\>**):
```c++
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
  }
}
```
## Solution 3: Re-Generate the object using it's constructor.
This is same principle as solution 2 but instead of modifying the string after we have default constructed, we pass a lambda that returns a tuple as a fourth parameter of **read_vector_from_file**, the tuple will be used by PickleJar to construct the object in place, as if you passed the contents of the tuple as parameters to the Non-TriviallyCopiable Type (using **write_vector_to_file** and **read_vector_from_file\<std::string\>**):
```c++
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
```
## Solution 4: Based in **Solution 3**. Ignore the value and instead use it's default constructor.
In this case we just default generate all the strings, which in turn will make all of them be empty strings (using **write_vector_to_file** and **read_vector_from_file\<std::string\>**):
```c++
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
```

## Explanation about Non-TriviallyCopiable Types
A Non-TriviallyCopiable type in plain terms is an object that is or contains strings, pointers, or other types that aren't stored directly inside the memory address range of it's object. For example, strings have a pointer to heap memory, if you want to copy a string you have to access it's .data() member to get a pointer to the character bytes.

If we try to use the same TriviallyCopiable code for Types with a Non-TriviallyCopiable type, PickleJar will give a compile time error:
```
picklejar.hpp:375:17: error: static assertion failed: PICKLEJAR_HELP: The object passed to picklejar is not trivially copiable. Use of the other versions of this function may be needed and then a call to 'preserve_blank_instance_member' and 'copy_new_bytes_to_instance' in the lambda to fix non trivially copiable members. SEE NON TRIVIAL EXAMPLES section in the readme file
  375 |   static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
      |                 ^~~~~~~~~~~~~~~~~~~~~~~
```
The reason for this warning is that if we were to copy the string bytes directly from the file into an instance of that type, we will need to have the following considerations:
1. In order to copy the bytes PickleJar has to instantiate an object of std::string type and then copy the bytes from the file into the new instance. Since we instantiated a std::string, that std::string will contain an address to memory to store our characters separate from the content of it's instance, this address is problematic because the address we stored in the file will not match the one we have in our new instance and therefore cause a leak and this would cause **Undefined Behavior**.
2. We have to store the bytes of the characters contained in the std::string(it's a container after-all) to the file
3. And also store the bytes that belong to the std::string in our new instance(that we know contains a valid address because we just created it on this run) and
4. Read the character bytes and add them into our new std::string instance that we created this run

That's a lot of work, luckily PickleJar has some convenience functions to assist on doing this correctly, but you will have to do the following first:
## Okay, so how does PickleJar deal with Non-TriviallyCopiable Types, explaning [Solution 4](#solution-4-based-in-solution-3-ignore-the-value-and-instead-use-its-default-constructor)
* If what you are trying to store is or contains a Non-TriviallyCopiable Type you will have to go to the class or struct declaration and take note of which members of the object would result in something that is not directly copiable, if it contains a string or a pointer you will need to take note of the name of those members and how it's bytes are stored. You then need to decide what you want to do with it [See this section to find out what you can do with each member](#what-can-we-do-for-non-triviallycopiable-types). However if you need a deep copy of your object you will have to use [Solution 1](#solution-1-copy-the-size-of-the-non-triviallycopiable-object-along-with-its-bytes-this-way-we-know-how-many-characters-to-readstore) this section deals with Solution 4, 2, 3(In that order you should try not to use solution 2 if possible).

With the name of the members that need special attention we can now proceed to the fixing itself:
Let's go back to our vector of std::strings, this code will give the error from the previous section:
```c++
  // THE FOLLOWING CODE WILL GIVE A COMPILE TIME ERROR AS EXPECTED
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",  "16",
                                      "32", "64", "128", "256", "512"};

  if (picklejar::write_vector_to_file(string_vec, "example1.data"))
    std::puts("WRITESUCCESS");

  if (auto optional_read_vector = picklejar::read_vector_from_file<std::string>(
          "example1.data");  // This results in compiler error because
                             // std::string is non-trivially-copiable
      optional_read_vector.has_value()) {
    std::puts(
        ("READSUCCESS: fifth element is " + optional_read_vector.value().at(4))
            .c_str());
  }
```
Now our first step is to preserve our blank instance valid string bytes, for this you can pass a lamda funtion as third parameter to ```picklejar::read_vector_from_file<std::string>(..., ..., operation_modify_using_previous_bytes)```, where ```operation_modify_using_previous_bytes``` is our lamda where we will fix this issue. This lambda is run for each element that will be placed in our new vector, and it takes 3 parameters:
1. **auto &blank_instance**: this is our new std::string instance we just created, we will have to copy the bytes from the file into this object.
2. **auto &valid_bytes_from_new_blank_instance**: this is a ```const char[]``` that contains a copy of the bytes of our blank instance.
3. **auto &bytes_from_file**: this is a ```const char[]``` that contains a copy of the bytes we got from our file.

```c++
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",  "16",
                                      "32", "64", "128", "256", "512"};

  if (picklejar::write_vector_to_file(string_vec, "example1.data"))
    std::puts("WRITESUCCESS");

  if (auto optional_read_vector = picklejar::read_vector_from_file<std::string>(
          "example1.data",
          [](auto &blank_instance,
             auto &valid_bytes_from_new_blank_instance,
             auto &bytes_from_file) {
            picklejar::util::preserve_blank_instance_member(
                0, sizeof(std::string), valid_bytes_from_new_blank_instance,
                bytes_from_file);
            picklejar::util::copy_new_bytes_to_instance(
                bytes_from_file, blank_instance,
                sizeof(std::string));
          });
      optional_read_vector.has_value()) {
    std::puts(("READSUCCESS: last_element=" +
               optional_read_vector.value().at(
                   optional_read_vector.value().size() - 1))
                  .c_str());
  }
```
In the previous code we used two ```picklejar::util``` functions to:
1. Preserve our new empty string with:
```c++
picklejar::util::preserve_blank_instance_member(
                0, sizeof(std::string), valid_bytes_from_new_blank_instance,
                bytes_from_file);
```

**preserve_blank_instance_member** takes the following parameters:
* **size_t blank_instance_member_offset**: a starting offset for the bytes we want to preserve.
* **size_t blank_instance_member_size**: a size that let us find out the range of bytes that need preserving.
* **auto &valid_bytes_from_new_blank_instance**: the byte copy of our new instance.
* **auto &bytes_from_file**: the byte copy we got from our file.
2. Copy our fixed bytes to our blank instance memory address, in this case nothing changes since we are "preserving" the original string in essense we have copied the same bytes the string had before:
```c++
picklejar::util::copy_new_bytes_to_instance(
                bytes_from_file, blank_instance,
                sizeof(std::string));
```
**copy_new_bytes_to_instance** takes the following parameters:
* **auto &bytes_to_copy_to_instance**: the bytes to copy into our blank instance
* **auto &blank_instance**: our new instance we created this run
* **size_t size_of_object**: the size of the bytes to copy into our instance

Now what we just did allows us to compile our code and it's what we did for [Solution 4 - Ignore the string](#solution-4-based-in-solution-3-ignore-the-value-and-instead-use-its-default-constructor), the strings in our vector will be all empty strings because we have kept our default strings by using the **picklejar::preserve_blank_instance_member** helper function.

## Explaining [Solution 2](#solution-2-dont-save-it-and-re-generate-the-non-triviallycopiable-object-when-we-run-the-program-again)
We have seen how to preserve our default constructed string, this may be desirable in some circumstances, but chances are that you may want to have something inside this string other than a default constructed value. Solution 2 and solution 3 deal with this problem, 
* Solution 3 passes parameters to the string constructor.
* Solution 2 allows the object to the created first with it's default constructor and THEN modifies it.

Both are fine, solution 2 will show a compile time error if the type used is not default constructible. Solution 3 may be more efficient in cases more complex than a string, because we would be constructing the object in-place without copy or move operations(In most cases).

Code-wise, Solution 2 is identical to Solution 4 except at the end of the **operation_modify_using_previous_bytes** lambda we modify the string with a generated value:
```c++
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
                bytes_from_file, blank_instance,
                sizeof(std::string));
            blank_instance = "string" + std::to_string(++count);
          });
      optional_read_vector.has_value()) {
    std::puts(("READSUCCESS: last_element=" +
               optional_read_vector.value().at(
                   optional_read_vector.value().size() - 1))
                  .c_str());
  }
```
## Explaining [Solution 3](https://github.com/tomasguillen/picklejar#solution-3-re-generate-the-object-using-its-constructor)
Code-wise, Solution 3 is identical to Solution 2 except we add a fourth parameter to the **read_vector_from_file** function called **constructor_generator**, which takes a lambda that returns a tuple with the parameters for the string constructor, this allows us to construct the object in-place. This is not impressive with a simple std::string but it will be more efficient for more complex objects and it will allow us to do this with objects that are not default constructible.
```c++
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
                bytes_from_file, blank_instance,
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
```

## What comes with PickleJar:
There are **3** types of **READ** operations (v1, v2, v3), and **1** type of **WRITE** operations (v1)
### The READ operation depends on the number of parameter passed:
* v1 = minimal version (only using the base parameters [see below](#base-parameters-for-objects-v1---minimal-version))
* v2 = with **operation_modify_using_previous_bytes** (an additional lambda passed that allows to manipulate the bytes that will be copied into the new object).
* v3 = with **operation_modify_using_previous_bytes** **constructor_generator** (an additional lambda passed that allows to construct the object by returning a lambda with the parameters for the constructor of each object).

You can use the algorithms with **objects** and **std::vector(or any container with fully sequential data, std::array)**:
Example:
* read_object_from_buffer: **for single objects**
* read_vector_from_buffer: **for std::vector or std::array**

### Base Parameters for objects (v1 - minimal version):
Picklejar algorithms for **objects** can read and write data from/to 3 different interfaces:
1. An existing **std::ifstream** passed as FIRST parameter, OR
2. A **filename** (uses std::ifstream) passed as FIRST parameter, OR
3. A **buffer of bytes**(char type) passed as FIRST parameter

### Base Parameters for vectors (v1 - minimal version):
Picklejar algorithms for **vectors** can read and write data from/to 3 different interfaces:
1. An existing **std::ifstream** passed as SECOND parameter, the container to push_back() the elements as FIRST parameter, OR
2. A **filename** (uses std::ifstream) passed as FIRST parameter, OR
3. A **buffer of bytes**(char type) passed as SECOND parameter, the container to push_back() the elements as FIRST parameter

### Explanation of lambda parameters (v2 - operation_modify_using_previous_bytes version)
Typically this is what you will do in this parameter:
```c++
          [](auto &blank_instance, auto &valid_bytes_from_new_blank_instance,
             auto &bytes_from_file) {
            picklejar::util::preserve_blank_instance_member(
                0, sizeof(std::string), valid_bytes_from_new_blank_instance,
                bytes_from_file);
            picklejar::util::copy_new_bytes_to_instance(
                bytes_from_file, blank_instance,
                sizeof(std::string));
          }
```
In this case we are basically not doing anything, we are copying the valid bytes from our std::string that we created on this application run to a buffer and then we are copying them back into our new instance, essentially not changing it's bytes at all, this is useful for [Solution 4 - Ignore the string](#solution-4-based-in-solution-3-ignore-the-value-and-instead-use-its-default-constructor) as explained above.
But you can also use ```picklejar::util::preserve_blank_instance_member``` with members of structs and classes:
```c++
                constexpr auto class_member_offset =
                    offsetof(UITransposeFilter, id);
                picklejar::util::preserve_blank_instance_member(
                    class_member_offset, sizeof(std::string),
                    valid_bytes_from_new_blank_instance, bytes_from_file);
```
In this case we are finding the byte offset of the member UITransposeFilter::id and preserving the bytes from the current application run in order to copy the other members into our new object. This allows us to ignore the id member if we don't care about copying it.

## GCC Compiler Flag caveat:
If compiling with GCC and have -Werror you may want to turn off -Wno-class-memaccess if it gives a warning but should only give warning when using picklejar with non-trivially-copiable objects

