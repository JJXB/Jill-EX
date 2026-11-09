// FUN_1353_000d @ 1353:000d

void __cdecl16far
FUN_1353_000d(uint *param_1,uint param_2,uint param_3,uint param_4,uint param_5,uint param_6,
             uint param_7,uint param_8)

{
  int iVar1;
  
  *param_1 = param_8;
  FUN_1353_0b20((uint *)param_1 + 0xc,param_1._2_2_,0);
  ((uint *)param_1)[0xc] = param_2 << 3;
  ((uint *)param_1)[0xd] = param_3;
  ((uint *)param_1)[0xe] = param_4 * 0x10 + 0x10;
  if ((param_8 & 2) == 0) {
    iVar1 = 0x1c;
  }
  else {
    iVar1 = 0x10;
  }
  ((uint *)param_1)[0xf] = param_5 * 0x10 + iVar1;
  FUN_1353_0b20((uint *)param_1 + 0x14,param_1._2_2_,0);
  ((uint *)param_1)[0x14] = param_2 * 8 + 8;
  if ((param_8 & 2) == 0) {
    iVar1 = 0x10;
  }
  else {
    iVar1 = 8;
  }
  ((uint *)param_1)[0x15] = iVar1 + param_3;
  ((uint *)param_1)[0x16] = param_4 << 4;
  ((uint *)param_1)[0x17] = param_5 << 4;
  if (param_6 != 0) {
    ((uint *)param_1)[0x16] = ((uint *)param_1)[0x16] - (param_6 * 0x10 + 8);
    ((uint *)param_1)[0x14] = ((uint *)param_1)[0x14] + param_6 * 0x10 + 8;
  }
  FUN_1353_0b20((uint *)param_1 + 0x1c,param_1._2_2_,8);
  ((uint *)param_1)[0x1c] = param_2 * 8 + 8;
  ((uint *)param_1)[0x1d] = param_3 + 0x10;
  ((uint *)param_1)[0x1e] = param_6 << 4;
  ((uint *)param_1)[0x1f] = param_7 * 0x10 + 5;
  FUN_1353_0b20((uint *)param_1 + 0x24,param_1._2_2_,8);
  ((uint *)param_1)[0x24] = param_2 * 8 + 8;
  ((uint *)param_1)[0x25] = param_7 * 0x10 + param_3 + 0x1b;
  ((uint *)param_1)[0x26] = param_6 << 4;
  ((uint *)param_1)[0x27] = (param_5 * 0x10 + param_7 * -0x10) - 0xb;
  ((uint *)param_1)[2] = param_2;
  ((uint *)param_1)[1] = param_2 << 3;
  ((uint *)param_1)[3] = param_3;
  ((uint *)param_1)[5] = param_4;
  ((uint *)param_1)[4] = param_4 << 4;
  ((uint *)param_1)[7] = param_5;
  ((uint *)param_1)[6] = param_5 << 4;
  ((uint *)param_1)[9] = param_6;
  ((uint *)param_1)[8] = param_6 << 4;
  ((uint *)param_1)[0xb] = param_7;
  ((uint *)param_1)[10] = param_7 << 4;
  return;
}


