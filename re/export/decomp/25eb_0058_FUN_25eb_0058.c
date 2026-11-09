// FUN_25eb_0058 @ 25eb:0058

void __cdecl16far FUN_25eb_0058(uint param_1)

{
  undefined2 uVar1;
  undefined2 uVar2;
  undefined1 local_12 [16];
  
  FUN_1353_074e(0xb83e,0x2cff,0,0,2,(char *)s_kind__2cff_168c,0x2cff);
  FUN_1353_074e(0xb83e,0x2cff,0x1e,0,2,
                ((undefined2 *)&DAT_2cff_b99c)
                [(char)((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f] * 2],
                ((undefined2 *)&DAT_2cff_b99e)
                [(char)((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f] * 2]);
  FUN_1353_074e(0xb83e,0x2cff,0,6,2,(char *)s_stat__2cff_169e,0x2cff);
  uVar2 = (undefined2)((ulong)param_1 * 0x1f >> 0x10);
  uVar1 = FUN_2b85_0085(*(undefined2 *)
                         ((int)(undefined2 *)&DAT_2cff_96b9 + (int)((ulong)param_1 * 0x1f)),local_12
                       );
  FUN_1353_074e(0xb83e,0x2cff,0x1e,6,2,uVar1,uVar2);
  FUN_1353_074e(0xb83e,0x2cff,0,0xc,2,0x16b0,0x2cff);
  uVar2 = (undefined2)((ulong)param_1 * 0x1f >> 0x10);
  uVar1 = FUN_2b85_0085(*(undefined2 *)
                         ((int)(undefined2 *)&DAT_2cff_96b1 + (int)((ulong)param_1 * 0x1f)),local_12
                       );
  FUN_1353_074e(0xb83e,0x2cff,0x1e,0xc,2,uVar1,uVar2);
  FUN_1353_074e(0xb83e,0x2cff,0,0x12,2,0x16c2,0x2cff);
  uVar2 = (undefined2)((ulong)param_1 * 0x1f >> 0x10);
  uVar1 = FUN_2b85_0085(*(undefined2 *)
                         ((int)(undefined2 *)&DAT_2cff_96b3 + (int)((ulong)param_1 * 0x1f)),local_12
                       );
  FUN_1353_074e(0xb83e,0x2cff,0x1e,0x12,2,uVar1,uVar2);
  FUN_1353_074e(0xb83e,0x2cff,0,0x18,2,0x16d4,0x2cff);
  uVar2 = (undefined2)((ulong)param_1 * 0x1f >> 0x10);
  uVar1 = FUN_2b85_0085(*(undefined2 *)
                         ((int)(undefined2 *)&DAT_2cff_96bf + (int)((ulong)param_1 * 0x1f)),local_12
                       );
  FUN_1353_074e(0xb83e,0x2cff,0x1e,0x18,2,uVar1,uVar2);
  if ((((undefined2 *)&DAT_2cff_be3c)[(char)((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f]] & 0x40)
      != 0) {
    FUN_1353_074e(0xb83e,0x2cff,0,0x1e,2,(char *)s_Text_Inside_2cff_16e6,0x2cff);
  }
  return;
}


