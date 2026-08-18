# MI-C Compiler 3.18I

This folder contains the MI-C compiler for CP/M that I use to program some CP/M tools and games.
It contains also my own libraries and sample code that I use in my projects.

It is a German C compiler from 1983 and uses old K&R syntax, not ANSI C.

The K&R (Kernighan & Ritchie) standard is the oldest C standard and it looks like this:
```
  // <- this is NOT allowed
  function(p1, p2, c)
    char *p1, *p2;
    int c
  {
    int a, b, c;
    ...
  }
```
All variables have to be declared at the beginning of a function. There is no "void" and the default return type is int.
// ist not allowed for comments. The library is very limited, which is why I am gradually expanding it.

But even this old C standard has a 32 bit long data type. So you do not have to use slow floating point types in most cases. And it has fast buffered file I/O which is both a big advantage over Turbo Pascal 3.0 for CP/M.

The MI-C compiler produces very small Z80 assembler code (8080 optinal) that can be assembled and linked with M80 and L80. It is very easy to mix C and assembler code.

I used this compiler in 1984 on a Sharp MZ80B with CP/M 2.2. No big projects, only some tools and some ROM code for small Z80 embedded systems.

**As with the entire repository, this is also a work in progress. Much is still unfinished or not yet properly tested.**

I'm testing by using it. I lack the time and patience for systematic testing. ;-)
