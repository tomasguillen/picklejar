/*v1 = minimal version
v2 = with operation_modify_using_previous_bytes
v3 = with constructor_generator
*/

// API STATUS:
// READ OPERATIONS: stream:complete,
// file:complete(maybe missing a v1 with constructor_generator?),
// object_stream:complete(maybe missing a v1 with constructor_generator?),
// object_buffer:complete(maybe missing a v1 with constructor_generator?),
// buffer:complete(maybe missing a v1 with constructor_generator?)

// object_stream_v1

// START object_file_v1 uses object_stream_v1

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
