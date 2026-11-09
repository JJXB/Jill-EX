// FUN_25d3_0000 @ 25d3:0000

void __cdecl16far FUN_25d3_0000(void)

{
  uint *puVar1;
  undefined4 uVar2;
  undefined2 uVar3;
  undefined2 uVar4;
  int iVar5;
  undefined2 in_DX;
  int local_8;
  uint local_6;
  char local_3;
  
  for (local_8 = 0; local_8 < 600; local_8 = local_8 + 1) {
    *(undefined2 *)(local_8 * 8 + -0x7c14) = 0x4700;
    *(undefined2 *)(local_8 * 8 + -0x7c0e) = 0x2cff;
    *(undefined2 *)(local_8 * 8 + -0x7c10) = 0x1688;
    *(undefined2 *)(local_8 * 8 + -0x7c12) = 0x4006;
  }
  uVar3 = FUN_2b31_003f((char *)s_jill_dma_2cff_1766,0x2cff,0x8000);
  while( true ) {
    iVar5 = FUN_2b58_000d(uVar3,&local_8);
    if (iVar5 < 1) break;
    FUN_2b58_000d(uVar3,local_8 * 8 + -0x7c14,0x2cff,2);
    FUN_2b58_000d(uVar3,&local_6);
    puVar1 = (uint *)(local_8 * 8 + -0x7c12);
    *puVar1 = *puVar1 ^ local_6;
    FUN_2b58_000d(uVar3,&local_3);
    uVar4 = FUN_2ae9_000a(local_3 + 1);
    *(undefined2 *)(local_8 * 8 + -0x7c0e) = in_DX;
    *(undefined2 *)(local_8 * 8 + -0x7c10) = uVar4;
    FUN_2b58_000d(uVar3,*(undefined2 *)(local_8 * 8 + -0x7c10),
                  *(undefined2 *)(local_8 * 8 + -0x7c0e),(int)local_3);
    uVar2 = *(undefined4 *)(local_8 * 8 + -0x7c10);
    *(undefined1 *)((int)uVar2 + (int)local_3) = 0;
  }
  for (local_8 = 0; local_8 < 6; local_8 = local_8 + 1) {
    ((undefined2 *)&DAT_2cff_be2c)[local_8] = 0;
  }
  DAT_2cff_be34 = DAT_2cff_be34 | 2;
  DAT_2cff_be2c = DAT_2cff_be2c | 1;
  DAT_2cff_be30 = DAT_2cff_be30 | 1;
  DAT_2cff_be36 = DAT_2cff_be36 | 2;
  return;
}


