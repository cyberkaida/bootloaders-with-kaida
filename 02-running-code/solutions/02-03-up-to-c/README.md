# Up to C

In this exercise we will move from raw assembly to executing C code.

To do this we will need to implement a few things that the loader[^1] normally
does for us.

- Set up the [stack](https://en.wikipedia.org/wiki/Call_stack)
- Perform the loader functions like clearing .bss
- Set up the [calling convention](https://en.wikipedia.org/wiki/Calling_convention) for C

## Extra reading

[^1]: [Wikipedia on Loaders](https://en.wikipedia.org/wiki/Loader_(computing))
