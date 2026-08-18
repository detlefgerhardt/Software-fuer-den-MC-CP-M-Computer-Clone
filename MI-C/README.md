# MI-C Compiler 3.18I

This folder contains the MI-C compiler for CP/M that I use to program some CP/M tools and games.
It contains also my own libraries and sample code that I use in my projects.

It is a German C compiler from 1983 and uses old K&R syntax, not ANSI C.

The K&R (Kernighan & Ritchie) is the oldest C standard. Function declarations look like this:

function(p1, p2, c)
  char *p1,*p2;
  int c
{
}

and all variables have to be declared at the beginning of a function. There is no void and no bool and the default return type is int.
// ist not alowed for comments.

The MI-C compiler produces very small Z80 assembler code (8080 optimal) that can be assembled and linked with M80 and L80. It is very eay to mix C and assembler code.

I used this compiler in 1984 on a Sharp MZ80B with CP/M 2.2. No big projects, only some tools and some ROM code for small Z80 embedded systems.
