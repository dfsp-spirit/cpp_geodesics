# The vv binary format for numeric matrices

This is a very basic custom file format for storing numeric data. The data must consist of N rows, where each row can have a different number of elements.

## Endianness

The file is always written in big endian byte order, independent of system endianness.

## Fields (in this order)

* signed 32 bit integer: file magic number. Always the value 42.
* signed 32 bit integer: DT, the data type for the whole matrix. one of: 13 (meaning int32) or 14 (meaning float32)
* signed 32 bit integer: N, the number of rows in the matrix.
* N times:
  - signed 32 bit integer: M, the number of elements in this row
  - M times DT: the data for this row.


## Source code

See [here](./src/common/write_data.h), function `write_vv`.
