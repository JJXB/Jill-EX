// FUN_22ab_1f69 @ 22ab:1f69

/* WARNING: Removing unreachable block (ram,0x00024cbf) */
/* WARNING: Removing unreachable block (ram,0x00024d06) */
/* WARNING: Removing unreachable block (ram,0x00024cf9) */
/* WARNING: Removing unreachable block (ram,0x00024d09) */
/* WARNING: Removing unreachable block (ram,0x00024d11) */
/* WARNING: Removing unreachable block (ram,0x00024d2e) */
/* WARNING: Removing unreachable block (ram,0x00024d21) */
/* WARNING: Removing unreachable block (ram,0x00024d0d) */
/* WARNING: Removing unreachable block (ram,0x00024d31) */
/* WARNING: Removing unreachable block (ram,0x00024d3b) */
/* WARNING: Removing unreachable block (ram,0x00024d42) */
/* WARNING: Removing unreachable block (ram,0x00024d4c) */
/* WARNING: Removing unreachable block (ram,0x00024d58) */
/* WARNING: Removing unreachable block (ram,0x00024d53) */
/* WARNING: Removing unreachable block (ram,0x00024d5d) */
/* WARNING: Removing unreachable block (ram,0x00024d49) */

void __cdecl16far
FUN_22ab_1f69(undefined2 param_1,undefined2 param_2,undefined2 param_3,undefined2 param_4)

{
  undefined2 uVar1;
  int iVar2;
  undefined2 in_DX;
  int local_12;
  undefined1 local_10 [12];
  char local_4 [2];
  
  DAT_2cff_3d90 = 1;
  DAT_2cff_3d92 = 1;
  DAT_2cff_3d9e = 1;
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,5,1);
  FUN_1353_0b4a(DAT_2cff_b6ae,DAT_2cff_b6b0);
  FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,0,4,1,(char *)s______________2cff_140d,0x2cff);
  FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,0,0x38,1,(char *)s______________2cff_141a,0x2cff);
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,4,1);
  FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,6,2,2,param_1,param_2);
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,3,1);
  for (local_12 = 0; local_12 < 6; local_12 = local_12 + 1) {
    uVar1 = FUN_2b85_0085(local_12 + 1,local_10);
    FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,8,local_12 * 8 + 0xd,2,uVar1,in_DX);
  }
  for (local_12 = 0; local_12 < 6; local_12 = local_12 + 1) {
    iVar2 = FUN_2b9a_000b((undefined1 *)&DAT_2cff_bab4 + local_12 * 0xc,0x2cff);
    if (iVar2 == 0) {
      FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,0x14,local_12 * 8 + 0xd,2,param_3,param_4);
    }
    else {
      FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,0x14,local_12 * 8 + 0xd,2,
                    (undefined1 *)&DAT_2cff_bab4 + local_12 * 0xc,0x2cff);
    }
  }
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,2,1);
  FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,0xe,0x41,2,(char *)s_PRESS_2cff_1427,0x2cff);
  FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,6,0x4d,2,(char *)s_TO_ABORT_2cff_142d,0x2cff);
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,4,1);
  FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,0xc,0x47,2,(char *)s_ESCAPE_2cff_1436,0x2cff);
  FUN_1353_0b64(DAT_2cff_b6ae,DAT_2cff_b6b0,7,1);
  local_4[1] = 0;
  FUN_1415_026f(0);
  local_12._0_1_ = ((byte)local_12 & 7) + 1;
  local_4[0] = (char)local_12;
  FUN_1353_074e(DAT_2cff_b6ae,DAT_2cff_b6b0,1,DAT_2cff_1244 * 8 + 0xd,2,local_4);
  do {
  } while( true );
}


