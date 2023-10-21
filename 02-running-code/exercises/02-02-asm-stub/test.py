#!/usr/bin/env python3
# python3 -m pip install rzpipe

import rzpipe

pipe = rzpipe.open("kernel8.img")
pipe.cmd('aa')

output = pipe.cmdj("pdj 1 @ 0x0")
print("Checking instruction count")
assert len(output) == 1, "Should only be one instruction"
print("Checking shellcode offset")
assert output[0]["offset"] == 0, "First instruction should be at offset 0 (No ELF header)"
print("Checking shellcode type")
assert output[0]["type"] == "jmp", "First instruction should be a jump"
print("Checking shellcode jump destination")
assert output[0]["jump"] == 0, "First instruction should jump to itself"

pipe.quit()
