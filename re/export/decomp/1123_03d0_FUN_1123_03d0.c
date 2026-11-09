// FUN_1123_03d0 @ 1123:03d0

void __cdecl16far
FUN_1123_03d0(int param_1,uint param_2,uint param_3,uint param_4,uint param_5,int param_6,
             int param_7,int param_8,int param_9)

{
  uint uVar1;
  uint uVar2;
  undefined2 unaff_SS;
  uint local_c [4];
  int local_4;
  
  local_c[0] = param_6 << 8 | param_2;
  local_c[1] = param_7 << 8 | param_3;
  local_c[2] = param_8 << 8 | param_4;
  local_c[3] = param_9 << 8 | param_5;
  for (uVar1 = 0; uVar1 < 0x100; uVar1 = uVar1 + 1) {
    uVar2 = 0;
    for (local_4 = 0; local_4 < 8; local_4 = local_4 + 2) {
      uVar2 = local_c[uVar1 >> ((byte)local_4 & 0x1f) & 3] << ((byte)local_4 & 0x1f) | uVar2;
    }
    ((undefined2 *)&DAT_2cff_3232)[param_1 * 0x100 + uVar1] = uVar2;
  }
  return;
}


