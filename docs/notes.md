# Notes on the implementation

- The .obj format allows two types declaration of indices:
    - positive integer: indexing relative to the start of the list,
    moving from the top of the list towards the current position (starting from 1)
    - negative integer: indexing relative to the current statement,
    moving from the current position towards the top of the list (starting from -1)
Also note that 1-base indexing is in use (in fact, an index with value 0 is invalid).
The current implementation normalizes the values to enable the use of 0-base indexing
which is predominant in the c/c++ world. Consistency checks are performed before the
normalization process to preserve compliance with the .obj specification.

- The implementation allows the usage of custom types to store parsed data,
specified with templates parameters. The type parameter `Value` expects a floating point type
and is used to store data points. The type `Index` expects an integer type
and is used to store index data.