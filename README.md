# Bootloaders with Kaida!

Welcome to Bootloaders with Kaida! This is a free class on writing low level
bootloader code and implementing secure boot! This is useful not only for
people writing bootloaders, but also for people curious about how computers
work or people who want to try low level development!

In this class you will learn about bootloaders, how secure boot is typically
implemented and about how to research hardware and develop a bootloader!

To run easily this class has some expectations. If you don't feel confident
in any of these areas, there are links to some resources that will help below!

Don't be afraid if you are not an expert in all these areas! The class material
includes a lot of external links to help refresh your memory or learn more!

- Familiarity with the C programming language[^2][^3]
  - Many bootloaders are written in C, so we will use C for our examples and exercises
  - You should know concepts such as pointers[^7], arrays[^8], functions[^9], heap and stack allocation
- Some basic knowledge of assembly[^4]
  - We will be writing some basic assembly to move values between registers[^5], set constants, etc
  - If you can write a for loop in assembly[^6] you will be OK!
  - If you are familiar with another architecture and not ARM, you will be OK!

The class is designed for the curious! You will find two kinds of links in the notes.
[Inline links like this](https://github.com/cyberkaida/bootloaders-with-kaida)
are for things you should read or understand for the material. You usually don't need to read
the entire thing, but the link is to something you will probably need.
The other kind of link is a footnote like this[^1], these are things that will improve your
knowledge or might be interesting, but are not required reading.

[^1]: A footnote is for things you might be interested in but is not required to read!

If you find something interesting please look through the footnotes or ask questions!
*Be curious and brave, these things are hard, there are no bad questions!*

I have streamed most of the development of this class on my [Twitch channel](https://twitch.tv/cyberkaida),
if you wonder "how can anyone know this?" you can watch as I learned these things
to put together this class and see the good and bad days.

This class has been a passion project for me, and I hope you make some supportive
friends, learn something interesting and have fun!

- 💜 サイバーカイダ

## Let's go!

1. [./01-prep-work](./01-prep-work)
2. [./02-running-code](./02-running-code)

## Reference material

Some reference material to help you get started!

[^2]: [Effective C](https://nostarch.com/Effective_C) - A book on C
[^3]: [Essential C](http://cslibrary.stanford.edu/101/) - An online C primer
[^4]: [ARM Assembler in Raspberry Pi](https://thinkingeek.com/arm-assembler-raspberry-pi/) - A guide to ARM assembly using the Raspberry Pi
[^5]: [ARM Assembler in Raspberry Pi - Registers](https://thinkingeek.com/2013/01/10/arm-assembler-raspberry-pi-chapter-2/)
[^6]: [ARM Assembler in Raspberry Pi - Control Structures](https://thinkingeek.com/2013/01/20/arm-assembler-raspberry-pi-chapter-6/)
[^7]: [Pointers](https://en.wikipedia.org/wiki/Pointer_(computer_programming))
[^8]: [Arrays](https://en.wikipedia.org/wiki/Array_(data_structure))
[^9]: [Functions](https://en.wikipedia.org/wiki/Function_(computer_programming))
[^10]: [Heap allocation](https://en.wikipedia.org/wiki/C_dynamic_memory_allocation)
[^11]: [Stack allocation](https://en.wikipedia.org/wiki/Call_stack)
