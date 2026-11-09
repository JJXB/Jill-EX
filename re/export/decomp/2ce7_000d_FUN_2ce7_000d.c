// FUN_2ce7_000d @ 2ce7:000d

uint __cdecl16far FUN_2ce7_000d(uint param_1)

{
  if (param_1 == 0xffff) {
    param_1 = 0xffff;
  }
  else if ((*(byte *)((param_1 & 0xff) + 0x2853) & 8) == 0) {
    param_1 = param_1 & 0xff;
  }
  else {
    param_1 = (param_1 & 0xff) - 0x20;
  }
  return param_1;
}


