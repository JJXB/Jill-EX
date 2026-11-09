// FUN_2c8f_000b @ 2c8f:000b

void __cdecl16far FUN_2c8f_000b(byte param_1,undefined4 param_2)

{
  undefined2 uVar1;
  undefined2 uVar2;
  char cVar3;
  undefined4 uVar4;
  undefined2 uVar5;
  undefined2 uVar6;
  int iVar7;
  undefined2 *puVar8;
  undefined2 uVar9;
  undefined2 uVar10;
  byte bVar11;
  byte bVar12;
  byte in_AF;
  byte bVar13;
  byte bVar14;
  byte in_TF;
  byte in_IF;
  byte bVar15;
  byte in_NT;
  undefined4 uVar16;
  undefined1 uStack0000;
  undefined2 *puVar17;
  code *local_14;
  undefined2 local_10;
  undefined1 local_e;
  undefined1 local_d;
  byte local_c;
  undefined1 local_b;
  undefined2 local_a;
  code *local_8;
  undefined1 local_6;
  undefined2 local_5;
  char cStack_3;
  
  local_14 = (code *)&local_10;
  local_10 = 0x6e8b;
  local_e = 0xe2;
  local_d = 0xcd;
  local_c = param_1;
  bVar11 = param_1 < 0x25;
  bVar15 = SBORROW1(param_1,'%');
  cVar3 = param_1 - 0x25;
  bVar14 = cVar3 < '\0';
  bVar13 = cVar3 == '\0';
  bVar12 = (POPCOUNT(cVar3) & 1U) == 0;
  if (!(bool)bVar11) {
    bVar11 = param_1 < 0x26;
    bVar15 = SBORROW1(param_1,'&');
    cVar3 = param_1 - 0x26;
    bVar14 = cVar3 < '\0';
    bVar13 = cVar3 == '\0';
    bVar12 = (POPCOUNT(cVar3) & 1U) == 0;
    if ((bool)bVar11 || (bool)bVar13) {
      local_b = 0x36;
      local_a = 0x68f;
      local_6 = 0xca;
      local_5 = 2;
      local_8 = local_14;
      goto LAB_2c8f_005c;
    }
  }
  local_b = 0xca;
  local_a = 2;
LAB_2c8f_005c:
  uVar9 = (undefined2)((ulong)param_2 >> 0x10);
  iVar7 = (int)param_2;
  uVar6 = *(undefined2 *)(iVar7 + 2);
  uVar5 = *(undefined2 *)(iVar7 + 4);
  puVar17 = (undefined2 *)CONCAT22(iVar7,(undefined2 *)*(undefined2 *)(iVar7 + 8));
  uVar2 = *(undefined2 *)(iVar7 + 10);
  uVar1 = *(undefined2 *)(iVar7 + 0x10);
  uVar4 = *(undefined4 *)(iVar7 + 0xc);
  uVar10 = (undefined2)((ulong)uVar4 >> 0x10);
  uVar16 = (*local_14)(0x2c8f,(undefined2 *)*(undefined2 *)(iVar7 + 8),iVar7,uVar9,
                       (uint)(in_NT & 1) * 0x4000 |
                       (uint)SBORROW2((int)register0x00000010 + -2,0x12) * 0x800 |
                       (uint)(in_IF & 1) * 0x200 | (uint)(in_TF & 1) * 0x100 |
                       (uint)((int)&local_14 < 0) * 0x80 |
                       (uint)(&uStack0000 ==
                             (char *)s_Turbo_C___Copyright__c__1988_Bor_2cff_0004 + 0x10) * 0x40 |
                       (uint)(in_AF & 1) * 0x10 |
                       (uint)((POPCOUNT((uint)&local_14 & 0xff) & 1U) == 0) * 4 |
                       (uint)((char *)((int)register0x00000010 + -2) <
                             (char *)s_Turbo_C___Copyright__c__1988_Bor_2cff_0004 + 0xe),0x2cff,
                       (int)register0x00000010 + -2);
  uVar9 = (undefined2)((ulong)puVar17 >> 0x10);
  puVar8 = (undefined2 *)puVar17;
  *puVar17 = (int)uVar16;
  puVar8[1] = uVar6;
  puVar8[2] = uVar5;
  puVar8[3] = (int)((ulong)uVar16 >> 0x10);
  puVar8[5] = uVar2;
  puVar8[8] = uVar1;
  puVar8[9] = (uint)(in_NT & 1) * 0x4000 | (uint)(bVar15 & 1) * 0x800 | (uint)(in_IF & 1) * 0x200 |
              (uint)(in_TF & 1) * 0x100 | (uint)(bVar14 & 1) * 0x80 | (uint)(bVar13 & 1) * 0x40 |
              (uint)(in_AF & 1) * 0x10 | (uint)(bVar12 & 1) * 4 | (uint)(bVar11 & 1);
  puVar8[4] = (int)register0x00000010 + -2;
  puVar8[6] = (int)uVar4;
  puVar8[7] = uVar10;
  FUN_2c8f_0052();
  return;
}


