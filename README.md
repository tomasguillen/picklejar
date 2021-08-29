# C++ Pickle Jar
Save and Load Objects and Vectors and Arrays from/to files, ifstreams or byte buffers; a simple versioning system prevents you from making common mistakes, and it allows you to update the objects stored after the fact.

## Two APIs:
* One for deep copying/reading with versioning and byte size redundancy
* And a different lower level API that just writes the bytes of the object; and has a number of read functions to account for simple and complex object caveats.

## Highlights:
1. Both APIs work with streams, files and arbitrary buffers of bytes.
2. You can mix both APIs to have redundancy and speed where needed: *deep_copy_*, and *deep_read_*, for reliability. And *write_*, and *read_*, for faster low level operations. The latter should only be used for objects that have sequential storage; unless, you plan to ignore non-sequential items. For example, a map doesn't have sequential storage so you would use *deep_\** API; but, if you have an object that contains a map, and you want to ignore the map, and read the other value members; the read API has a way to do that. See [Solution 4](#solution-4-based-in-solution-3-ignore-the-value-and-instead-use-its-default-constructor).

## Versioning System
Only the deep copy/read API is setup to be able to write versioned objects and vectors, you can see a complete example that uses all the capabilites of this library in *examples/versioning_example.cpp* and *examples/versioning_example_2.cpp*, the second is a copy of the first with one more step and they have very similar usage.

If you are trying to store non trivial types, I highly recommend you compile the versioning examples; since they pretty much show the whole point of this library. You may want to modify the code, to store and write the structures you are interested in; just look at how it works for the IntBasedString struct in the *versioning_example* files. A unit test is setup here: *tests/versioning_example_2_with_tests.cpp*; you can use it as a template for the structures you are interested in. It uses a thirdparty unit test library which is located in the *tests/ut* directory. It should be easy to change it to a different test framework. It's always nice to have tests setup for this type of thing; because, it allows to detect any changes that may need to be fixed in case anything outside your control changes. Ex: standard or compiler changes that can't be anticipated.

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

If you run step1, then step2. The program will first try to read the file with it's *version2 read function*, but it will fail and then try with it's *version1 to version2 conversion function* and succeed:
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

If you run step1, followed by step2, and then followed by step2 again, in the last run, the program will no longer have to convert from version 1 to version 2 and you won't see that warning message:
```
Attempting to read vector from file with 'step2_v2_read_function'
... Constructor Output ...
READ_SUCCESS_V2
```

If you run step1, followed by step2, and then followed by step3 it will read everything just fine, it will only show an error if you run step1 and then step3 because the version won't match as seen previously.

If you run step1, followed by step2, and then followed by step4 (step3 doesn't matter here), you will get behavior similar to what happens with step2 but with the version 2 being translated to version 4. If you then try to run step2 you will get 2 warnings, one for the *version 2 read function* and another for the *version 1 to version 2 conversion function* which is just what was intended.

###### The versioning unit tests are located in *tests/versioning_example_2_with_tests.cpp*
It runs all the steps in the order they are meant to be tested. It can be used as a template to create a unit tests for your own structures that you want to save and load to a file. I recommend to add a new test every time you make lasting changes to the saved structs you are planning to release.

# *write_\** and *read_\** API
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
2. Solution 2: Don't save it and Re-Generate the Non-TriviallyCopiable object when we run the program again.
3. Solution 3: Re-Generate the object using it's constructor.
4. Solution 4: Based in **Solution 3**. Ignore the value and instead use it's default constructor.
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
This Solution works if you can generate the object without much effort and the non-triviallycopiable object has a default constructor(otherwise see Solution 3 which is prefered to Solution 2), in other words you don't need an exact copy because the data can be generated (using **write_vector_to_file** and **read_vector_from_file<std::string>**):
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
    hexer::print_vec(optional_read_vector.value());
  }
}
```
## Solution 3: Re-Generate the object using it's constructor.
This is same principle as solution 2 but instead of modifying the string after we have default constructed, we pass a lambda that returns a tuple as a fourth parameter of **read_vector_from_file**, the tuple will be used by PickleJar to construct the object in place, as if you passed the contents of the tuple as parameters to the Non-TriviallyCopiable Type (using **write_vector_to_file** and **read_vector_from_file<std::string>**):
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
In this case we just default generate all the strings, which in turn will make all of them be empty strings (using **write_vector_to_file** and **read_vector_from_file<std::string>**):
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
A Non-TriviallyCopiable type in plain terms is an objects that is or contains strings, pointers, or other types that aren't stored directly inside the memory address range of it's object. For example, strings have a pointer to heap memory, if you want to copy a string you have to access it's .data() member to get a pointer to the character bytes.

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
    hexer::print_vec(optional_read_vector.value());
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
    hexer::print_vec(optional_read_vector.value());
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
### So which algorithm do I use?
|   | Second Header |
| ------------- | ------------- |
| Content Cell  | Content Cell  |
| Content Cell  | Content Cell  |

NON TRIVIAL EXAMPLES Section

## GCC Compiler Flag caveat:
If compiling with GCC and have -Werror you may want to turn off -Wno-class-memaccess if it gives a warning but should only give warning when using picklejar with non-trivially-copiable objects


// API STATUS:
// READ OPERATIONS: stream:complete,
// file:complete
// object_stream:complete
// object_buffer:complete
// buffer:complete

// object_stream_v1

// object_stream_v1_copy uses object_stream_v1

// START object_file_v1 uses object_stream_v1

// START object_file_v1_copy uses object_stream_v1

// stream_v1 uses object_stream_v1
  
// file_v1 uses stream_v1

// object_stream_v2

// object_file_v2 uses object_stream_v2

// object_buffer_v2

// object_stream_v3 uses object_stream_v2

// object_file_v3 uses object_stream_v2
// 
// object_buffer_v3 uses object_buffer_v2

// object_buffer_v1

// START object_buffer_v1_copy uses object_buffer_v1

// buffer_v1 uses object_buffer_v1

// buffer_v3 uses object_buffer_v2

// stream_v3 uses object_stream_v2

// stream_v2 uses stream_v3

// buffer_v2 uses buffer_v3

// file_v2 uses stream_v2

// file_v3 uses stream_v3


// WRITE OPERATIONS:

// START WRITE_VECTOR_TO

// object_stream_v1

// object_file_v1

// object_buffer_v1_array

// object_buffer_v1_vector

// stream_v1

// file_v1

// buffer_v1_array

// buffer_v1_vector
