# Huffman Tree

## Introduction

This is a simple implementation of Huffman tree in C. It comes with a simple command line interface and an HTTP server mode for encoding and decoding files.

## Compilation

```bash
make
```

## Usage

```sh
./main -h

Usage: ./main [OPTIONS]
Options:
  -c, --compress        Compress the input file
  -d, --decompress      Decompress the input file
  -i, --input <file>    The input file
  -o, --output <file>   The output file
  -h, --help            Print this message
  -s, --server          Run in server mode
```

## Server mode

Server mode is a simple HTTP server that can be used to encode and decode files. It takes users' input from url parameters and data forms to compress or decompress files. After either operation is done, users can download them to check the results. Since this is just a simple implementation of huffman tree, I hard-code variables like `port` and `BUFFER_SIZE`, i.e. Should you want to change anything, check the defined macros in main.c.
