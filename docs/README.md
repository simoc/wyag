# wyag

[![Build Status](https://travis-ci.org/simoc/wyag.svg?branch=master)](https://travis-ci.org/simoc/wyag)

[Write yourself a Git!](https://wyag.thb.lt/) tutorial
implemented in C++ instead of Python.

## Available Commands

Create files for a new Git Repository

```
wyag init
```

Dump contents of an object in the Git Repository

```
wyag cat-file blob 052d27891c3f624820a6c4acaa7db061a76c89f5

```

Create a hash for a file

```
wyag hash-object /etc/hosts
```

Create a PDF file with a diagram of a commit object
in the Git Repository

```
wyag log 4e8eab32b0e10fccc43c9279c318820e41a1ece8 > log.dot
dot -O -Tpdf log.dot
```

List the contents of a tree object in the Git Repository

```
wyag ls-tree 020f7a40c303e27becb029e68311a4e1070a52c1
```

Checkout files from a commit object in the Git Repository into a
new directory

```
wyag checkout 5d0ad40e8048d5dff14f5c6871e1aace51e12cfe /tmp/dir1
```
