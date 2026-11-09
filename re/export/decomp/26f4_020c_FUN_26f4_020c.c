// FUN_26f4_020c @ 26f4:020c

void __cdecl16far FUN_26f4_020c(uint param_1)

{
  undefined2 uVar1;
  undefined2 uVar2;
  int iVar3;
  undefined1 local_a [8];
  
  iVar3 = 0;
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) =
       ((undefined2 *)&DAT_2cff_b6b6)[(char)((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f]];
  *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96b7 + param_1 * 0x1f) =
       ((undefined2 *)&DAT_2cff_b740)[(char)((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f]];
  if (*(int *)((int)(undefined2 *)&DAT_2cff_96c3 + param_1 * 0x1f) != 0 ||
      *(int *)((int)(undefined2 *)&DAT_2cff_96c5 + param_1 * 0x1f) != 0) {
    iVar3 = FUN_2b9a_000b(*(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c3 + param_1 * 0x1f),
                          *(undefined2 *)((int)(undefined2 *)&DAT_2cff_96c5 + param_1 * 0x1f));
  }
  if (((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f] == '\x14') {
    *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) = iVar3 * 6;
  }
  else if (((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f] == '\x15') {
    *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) = iVar3 << 3;
  }
  else if (((undefined1 *)&DAT_2cff_96ac)[param_1 * 0x1f] == '\x1b') {
    uVar2 = (undefined2)((ulong)param_1 * 0x1f >> 0x10);
    uVar1 = FUN_2b85_0085(*(undefined2 *)
                           ((int)(undefined2 *)&DAT_2cff_96b9 + (int)((ulong)param_1 * 0x1f)),
                          local_a);
    iVar3 = FUN_2b9a_000b(uVar1,uVar2);
    *(int *)((int)(undefined2 *)&DAT_2cff_96b5 + param_1 * 0x1f) = (iVar3 + 2) * DAT_2cff_b6ec;
  }
  return;
}


