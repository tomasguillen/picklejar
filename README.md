# C++ Pickle Jar
Save and Load Objects and Vectors and Arrays from/to files, ifstreams or byte buffers.


## How to use PickleJar to store and recall TriviallyCopiable Types (simple ints, floats, doubles, structs...)
This is how to write a vector of ints to a file named "example1.data":
```
  std::vector<int> int_vec{0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

  if (picklejar::write_vector_to_file(int_vec, "example1.data"))
    std::puts("WRITESUCCESS");
```
```write_vector_to_file``` returns true if writting to the file was successful.
And this is how we can read it from that same file:
```
  if (auto optional_read_vector =
          picklejar::read_vector_from_file<int>("example1.data");
      optional_read_vector) {
    std::puts(("READSUCCESS: fourth element is " +
               std::to_string(optional_read_vector.value().at(4)))
                  .c_str());
  }
```
```read_vector_from_file<int>``` will return an object of type ```std::optional<std::vector>```. The if statement checks if there was a problem reading the file and to get our new vector we use ```optional_read_vector.value()```
Putting these two together we get the following output:
```
WRITESUCCESS
READSUCCESS: fourth element is 8
```
Notice how we passed ```<int>``` as a template parameter to the read function, this is because we stored ints in the **write_vector_to_file** operation.

## How to use PickleJar to store and recall Non-TriviallyCopiable Types (objects that are or contain strings, pointers, other types that aren't stored directly in the object memory address range)
If we try to use the same TriviallyCopiable code for Types with a Non-TriviallyCopiable type, PickleJar will give a compile time error:
```
picklejar.hpp:375:17: error: static assertion failed: PICKLEJAR_HELP: The object passed to picklejar is not trivially copiable. Use of the other versions of this function may be needed and then a call to 'preserve_blank_instance_member' and 'copy_new_bytes_to_instance' in the lambda to fix non trivially copiable members. SEE NON TRIVIAL EXAMPLES section in the readme file
  375 |   static_assert(TriviallyCopiable<Type>, TRIVIALLYCOPIABLE_MSG);
      |                 ^~~~~~~~~~~~~~~~~~~~~~~
```
The reason for this warning is that if we were to copy the string bytes directly from the file into an instance of that type, we will need to have the following considerations:
1. In order to copy the bytes PickleJar has to instantiate an object of std::string type and then copy the bytes from the file into the new instance. Since we instantiated a std::string, that std::string will contain an address to memory to store our characters separate from the content of it's instance, this address is problematic because the address we stored in the file will not match the one we have in our new instance and therefore cause a leak and this would cause **Undefined Behavior** and in some cases even **DEATH!!!** **You have been warned!!! :O ;)**
2. We have to store the bytes of the characters contained in the std::string(it's a container after-all) to the file
3. And also store the bytes that belong to the std::string in our new instance(that we know contains a valid address because we just created it on this run) and
4. Read the character bytes and add them into our new std::string instance that we created this run

That's a lot of work, luckily PickleJar has some convenience functions to assist on doing this correctly, but you will have to do the following first:
## Okay, so how do I use PickleJar with Non-TriviallyCopiable Types
* If what you are trying to store is a Non-TriviallyCopiable Type you will have to go to the class or struct declaration and take note of which members of the object would result in something that is not directly copiable, if it contains a string or a pointer you will need to take note of the name of those members.

With the name of the members that need special attention we can now proceed to the fixing itself:
Let's go back to our vector of std::strings, this code will give the error from the previous section:
```
  std::vector<std::string> string_vec{"0",  "1",  "2",   "4",   "8",  "16",
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
  }
```
Now our first step is to preserve our blank instance valid string bytes, for this you can pass a lamda funtion as third parameter to ```picklejar::read_vector_from_file<std::string>(..., ..., operation_modify_using_previous_bytes)```, where ```operation_modify_using_previous_bytes``` is our lamda where we will fix this issue. This lambda is run for each element that will be placed in our new vector, and it takes 3 parameters:
1. **auto &blank_instance**: this is our new std::string instance we just created, we will have to copy the bytes from the file into this object.
2. **auto &valid_bytes_from_new_blank_instance**: this is a ```const char[]``` that contains a copy of the bytes of our blank instance.
3. **auto &bytes_from_file**: this is a ```const char[]``` that contains a copy of the bytes we got from our file.

```
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
1. preserve our new blank instance with:
```
picklejar::util::preserve_blank_instance_member(
                0, sizeof(std::string), valid_bytes_from_new_blank_instance,
                bytes_from_file);
```
**preserve_blank_instance_member** takes the following parameters:
a. **size_t blank_instance_member_offset**: a starting offset for the bytes we want to preserve.
b. **size_t blank_instance_member_size**: a size that let us find out the range of bytes that need preserving.
c. **auto &valid_bytes_from_new_blank_instance**: the byte copy of our new instance.
d. **auto &bytes_from_file**: the byte copy we got from our file.
2. copy our fixed bytes to our blank instance:
```
picklejar::util::copy_new_bytes_to_instance(
                valid_bytes_from_new_blank_instance, blank_instance,
                sizeof(std::string));
```
**copy_new_bytes_to_instance** takse the following parameters:
a. **auto &bytes_to_copy_to_instance**: the bytes to copy into our blank instance
b. **auto &blank_instance**: our new instance we created this run
c. **size_t size_of_object**: the size of the bytes to copy into our instance


## What comes with PickleJar:
There are **3** types of **READ** operations (v1, v2, v3), and **1** type of **WRITE** operations (v1)
### The READ operation types are:
1. v1 = minimal version
2. v2 = with **operation_modify_using_previous_bytes**
3. v3 = with **operation_modify_using_previous_bytes** **constructor_generator**

You can use the algorithms in **objects** and **std::vector(or any container with fully sequential data, std::array)**, you will have to call the appropriately named function:
Example:
* read_object_from_buffer **for objects**
* read_vector_from_buffer **for std::vector or std::array**

###Using PickleJar with objects:
Picklejar algorithms for **objects** can read and write data from/to 3 different interfaces:
1. An existing **std::ifstream** passed as first parameter
2. A **filename** (uses std::ifstream) passed as first parameter
3. A **buffer of bytes**(char type) passed as first parameter

###Using PickleJar with vectors:
Picklejar algorithms for **vectors** can read and write data from/to 3 different interfaces:
1. An existing **std::ifstream** passed as second parameter, the container to push_back() the elements as second parameter
2. A **filename** (uses std::ifstream) passed as first parameter
3. A **buffer of bytes**(char type) passed as first parameter, the container to push_back() the elements as second parameter

### So which algorithm do I use?
|   | Second Header |
| ------------- | ------------- |
| Content Cell  | Content Cell  |
| Content Cell  | Content Cell  |

NON TRIVIAL EXAMPLES Section

If compiling with GCC and have -Werror you may want to turn off -Wno-class-memaccess if it gives a warning but should only give warning when using picklejar with non-trivially-copiable objects


// API STATUS:
// READ OPERATIONS: stream:complete,
// file:complete(maybe missing a v1 with constructor_generator?),
// object_stream:complete(maybe missing a v1 with constructor_generator?),
// object_buffer:complete(maybe missing a v1 with constructor_generator?),
// buffer:complete(maybe missing a v1 with constructor_generator?)

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
