// FUN_1123_0a9b @ 1123:0a9b

void __cdecl16far FUN_1123_0a9b(void)

{
  byte extraout_AH;
  byte extraout_AH_00;
  byte extraout_AH_01;
  undefined2 in_DX;
  int iVar1;
  uint local_16;
  int local_14;
  
  local_16 = 0xf00;
  FUN_2c8f_000b(0x10,&local_16);
  DAT_2cff_3a42 = local_16 & 0xff;
  DAT_2cff_3a44 = 0;
  DAT_2cff_3a4e = 0;
  DAT_2cff_3a50 = 0;
  DAT_2cff_3a4b = 0;
  DAT_2cff_3a49 = 0;
  DAT_2cff_3a32 = 0;
  DAT_2cff_3a34 = 0;
  DAT_2cff_3a36 = 0x140;
  DAT_2cff_3a38 = 200;
  DAT_2cff_3a3a = 0;
  DAT_2cff_3a3c = 0;
  if (DAT_2cff_3a4d == '\0') {
    local_16 = 4;
    FUN_2c8f_000b(0x10,&local_16);
    local_16 = 0xb00;
    local_14 = 1;
    FUN_2c8f_000b(0x10,&local_16);
    in_DX = 0x3d9;
    out(0x3d9,0x11);
    FUN_1123_05ae();
    DAT_2cff_3a4d = '\0';
    DAT_2cff_3a52 = 4;
    FUN_11f4_0e77((undefined2 *)&DAT_2cff_3a32,0x2cff,(uint)extraout_AH << 8);
  }
  else if (DAT_2cff_3a4d == '\x02') {
    DAT_2cff_3a47 = 0x2000;
    DAT_2cff_3a4d = '\x02';
    local_16 = 0x1200;
    local_14 = 0x31;
    FUN_2c8f_000b(0x10,&local_16);
    for (iVar1 = 0; iVar1 < 0x10; iVar1 = iVar1 + 1) {
      local_16 = 0x1000;
      local_14 = iVar1 * 0x101;
      FUN_2c8f_000b(0x10,&local_16);
    }
    local_16 = 0xd;
    FUN_2c8f_000b(0x10,&local_16);
    FUN_1123_063d();
    FUN_1123_04eb(1,9,0);
    DAT_2cff_3a52 = 8;
    FUN_11f4_0e77((undefined2 *)&DAT_2cff_3a32,0x2cff,(uint)extraout_AH_00 << 8);
  }
  else if (DAT_2cff_3a4d == '\x04') {
    DAT_2cff_3a47 = 0x4000;
    DAT_2cff_3a4d = '\x04';
    for (iVar1 = 0; iVar1 < 0x10; iVar1 = iVar1 + 1) {
      local_16 = 0x1000;
      local_14 = iVar1 * 0x101;
      FUN_2c8f_000b(0x10,&local_16);
    }
    local_16 = 0x1200;
    local_14 = 0x31;
    FUN_2c8f_000b(0x10,&local_16);
    local_16 = 0x13;
    FUN_2c8f_000b(0x10,&local_16);
    FUN_1123_0674();
    FUN_1123_0516(0x2a,0x22,0);
    DAT_2cff_3a52 = 1;
    FUN_1123_08cd();
    FUN_2ca4_000c(0x3c4,0x604);
    FUN_2ca4_000c(0x3ce,0x4005);
    FUN_2ca4_000c(0x3d4,0x14);
    FUN_2ca4_000c(0x3d4,0xe317);
    FUN_11f4_0e77((undefined2 *)&DAT_2cff_3a32,0x2cff,(uint)extraout_AH_01 << 8);
    FUN_1123_0791();
  }
  DAT_2cff_322e = FUN_2ae9_000a(1);
  DAT_2cff_3230 = in_DX;
  return;
}


