-- Copyright (c) 2023 Adrian "asie" Siekierka
--
-- Licensed under the Apache License, Version 2.0 with LLVM Exceptions,
-- See https://github.com/llvm-mos/llvm-mos-sdk/blob/main/LICENSE for license
-- information.

print("SECTIONS {")
for i=0,255 do
  print(string.format("  .rom_poke_table_%d : {", i))
  if i == 0 then
    print("    __rom_poke_table = .;")
  end
	print(string.format("    FILL(0x%08x)", i * 0x01010101))
	print(string.format("    . = . + (__rom_poke_table_size > %d ? 1 : 0);", i))
  print("  } >c_readonly")
end
print("}")
