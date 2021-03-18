# LLVM-MOS SDK
A WIP SDK for developing with the llvm-mos compiler.
## Project Status

At present, only one configuration for the Commodore 64 is supported, but
effort is being made to ensure that the SDK generalizes. Before the first
official release, several other first-class targets will be created to
provide examples to aid our users to port to other platforms.

A hello world example application can be built for the C64, but much of the
expected runtime is missing. As the compile approaches C99 compatibility,
this will fill out to become a full frestanding C99 implementation.

## Build

Building the SDK first requires a working LLVM-MOS compiler. There isn't an
official release yet, but if you're eager to try it out, you can compile it yourself.

### Install ninja

For the steps below to work as-is, you'll need to install Ninja, the fast,
parallel build tool favored by LLVM developers. Instructions for your
platform will vary; see https://ninja-build.org/.

Alternatively, you can set -G "Makefile" in each CMake command to use
standard UNIX Makefiles, or you can substitute any other CMake-supported
generator. Your compile times may take a hit, and LLVM is already very slow
to build, so Ninja is highly recommended.

### Build LLVM-MOS

See https://github.com/llvm-mos for more information.

```console
$ cd
$ git clone https://github.com/llvm-mos/llvm-mos.git
$ cd llvm-mos
$ mkdir build
$ cd build
$ cmake -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="MOS" \
        -DLLVM_TARGETS_TO_BUILD="" \
        -DLLVM_ENABLE_PROJECTS="clang;lld" \
        ../llvm
$ ninja clang lld llvm-ar llvm-ranlib
$ alias clang-mos=$HOME/llvm-mos/build/bin/clang
```

This will place binaries in a newly-created "bin" directory. An alias will
allow easily using clang later.

### Build LLVM-MOS-SDK

Once you have a working compiler, you can use it to build the SDK itself.
You'll need to point the SDK at the compiler using -DLLVM_MOS, which requires
an absolute path to the bin directory. If you compiled the compiler somewhere else, make sure to modify the snippet below with the correct path.

```console
$ cd ../..
$ git clone https://github.com/llvm-mos/llvm-mos-sdk.git
$ cd llvm-mos-sdk
$ mkdir build
$ cd build
$ cmake -DLLVM_MOS=$HOME/llvm-mos/build/bin -G "Ninja" ..
$ ninja
```

The SDK will now be present in the build (current) directory.

### Compile the example

Once you have the SDK, you can compile the sample program using the alias created earlier. You'll need to use `--sysroot` to tell clang where the SDK is; this will go away once a release/install solution is created.

```console
$ cd ..
$ clang-mos --target=mos-commodore-64 --sysroot=build -Os -flto -o hello.prg examples/hello_world.c

$ cat examples/hello_world.c
int main(void) {
  const char *cur = "HELLO, WORLD!\n";
  while (*cur)
    __chrout(*cur++);
  return 0;
}

$ hexdump -C hello.prg
00000000  01 08 0b 08 5d 1e 9e 32  30 36 31 00 00 00 4c 10  |....]..2061...L.|
00000010  08 a2 00 a9 48 20 d2 ff  bd 25 08 e8 e0 0e d0 f5  |....H ...%......|
00000020  a9 00 a2 00 60 48 45 4c  4c 4f 2c 20 57 4f 52 4c  |....`HELLO, WORL|
00000030  44 21 0a 00                                       |D!..|
00000034
```

### Clang Command Line Breakdown

Here's what's going on in the clang command line:

<dl>
  <dt><code>--target=mos-commodore-64</code></dt>
  <dd>
    Target the Commodore 64. Target "triples" are turned directly into the
    set of directories where headers, libraries, and linker scripts are
    expected to be in the built SDK. Each component of the triple after the
    <code>mos</code> becomes a directory in the path, relative to the
    sysroot.
  </dd>
  <dt><code>--sysroot=build</code></dt>
  <dd>
    Identify the root directory of the built SDK. The target-specific path
    from the triple is relative to this path.
  </dd>
  <dt><code>-flto</code></dt>
  <dd>
    Generate final assembly code in the linker, not in the compiler. The
    LLVM-MOS SDK ships its libraries as LLVM bitcode files, not as machine
    code object files, allowing the linker to generate and optimize code as a
    whole. In this example, this causes the <code>__chrout</code> routine to
    be fully inlined into the main routine and deleted from the resulting
    binary. This won't be strictly required in the future, but today,
    omitting it causes the emission of worse enough assembly that
    <code>hello_world.c</code> walks into unimplemented regions of the C
    runtime.
  </dd>
</dl>