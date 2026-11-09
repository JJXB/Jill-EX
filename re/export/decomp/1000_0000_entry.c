// entry @ 1000:0000

/* WARNING: Stack frame is not setup normally: Input value of stackpointer is not used */
/* WARNING: This function may have set the stack pointer */

void entry(void)

{
  int *piVar1;
  undefined1 *puVar2;
  code *pcVar3;
  code *pcVar4;
  int *piVar5;
  char cVar6;
  byte bVar7;
  uint uVar8;
  int iVar9;
  char cVar10;
  undefined2 uVar11;
  undefined2 extraout_DX;
  undefined2 extraout_DX_00;
  int iVar12;
  undefined1 *puVar13;
  int *piVar14;
  undefined1 *puVar15;
  undefined1 *puVar16;
  undefined1 *puVar17;
  undefined1 *puVar18;
  int unaff_SI;
  int *piVar19;
  int *piVar20;
  uint uVar21;
  int iVar22;
  undefined1 *puVar23;
  undefined2 unaff_ES;
  undefined2 uVar24;
  undefined4 uVar25;
  
  puVar13 = (undefined1 *)0xe6;
                    /* WARNING: Read-only address (ram,0x000101c7) is written */
  uRam000101c7 = 0x2cff;
  pcVar4 = (code *)swi(0x21);
  uVar25 = (*pcVar4)();
  uVar24 = s_Turbo_C___Copyright__c__1988_Bor_2cff_0004._40_2_;
  iVar9 = DAT_2cff_0002;
  uVar11 = (undefined2)((ulong)uVar25 >> 0x10);
  *(undefined2 *)&DAT_2cff_007d = (int)uVar25;
  *(undefined2 *)&DAT_2cff_007b = unaff_ES;
  *(undefined2 *)(char *)&DAT_2cff_0077 = uVar24;
  *(int *)&DAT_2cff_0091 = iVar9;
  *(undefined2 *)&DAT_2cff_0081 = 0xffff;
  *(undefined2 *)(puVar13 + -2) = 0x2e;
  FUN_1000_012f();
  piVar5 = (int *)*(undefined4 *)&DAT_2cff_0075;
  uVar24 = (undefined2)((ulong)piVar5 >> 0x10);
  piVar19 = (int *)piVar5;
  uVar8 = 0x7fff;
  piVar20 = piVar19;
  do {
    if (((*piVar20 == 0x3738) && (iVar12 = piVar20[1], (char)iVar12 == '=')) &&
       (*(int *)&DAT_2cff_0081 = *(int *)&DAT_2cff_0081 + 1,
       ((byte)((uint)iVar12 >> 8) & 0xdf) == 0x59)) {
      *(int *)&DAT_2cff_0081 = *(int *)&DAT_2cff_0081 + 1;
    }
    do {
      if (uVar8 == 0) break;
      uVar8 = uVar8 - 1;
      piVar1 = piVar20;
      piVar20 = (int *)((int)piVar20 + 1);
    } while ((char)piVar5 != (char)*piVar1);
    if (uVar8 == 0) goto LAB_1000_0099;
    piVar19 = (int *)((int)piVar19 + 1);
  } while ((char)*piVar20 != (char)piVar5);
  *(int *)&DAT_2cff_0075 = -(uVar8 | 0x8000);
  *(uint *)&DAT_2cff_0079 = (int)piVar19 * 4 + 0x10U & 0xfff0;
  uVar8 = *(uint *)&DAT_2cff_1208;
  if (uVar8 < 0x200) {
    uVar8 = 0x200;
    *(undefined2 *)&DAT_2cff_1208 = 0x200;
  }
  bVar7 = 4;
  uVar21 = (uVar8 >> 4) + 1;
  if (uVar21 <= iVar9 + 0xc70bU) {
    iVar12 = (uVar8 >> 4) + 0x38f6;
    *(int *)&DAT_2cff_0089 = iVar12;
    *(int *)((int)(undefined4 *)&DAT_2cff_008b + 2) = iVar12;
    iVar12 = iVar12 - *(int *)&DAT_2cff_007b;
    piVar14 = (int *)(puVar13 + -2);
    *(uint *)(puVar13 + -2) = uVar21;
    pcVar4 = (code *)swi(0x21);
    (*pcVar4)();
    iVar22 = *piVar14 << (bVar7 & 0x1f);
    puVar23 = (undefined1 *)&DAT_2cff_2a5c;
    for (iVar9 = -0x6b08; iVar9 != 0; iVar9 = iVar9 + -1) {
      puVar2 = puVar23;
      puVar23 = puVar23 + 1;
      *puVar2 = 0;
    }
    *(undefined2 *)(iVar22 + -2) = 0x1000;
    pcVar3 = (code *)*(undefined2 *)&DAT_2cff_2a46;
    puVar15 = (undefined1 *)(iVar22 + -4);
    *(undefined2 *)(iVar22 + -4) = 0xd3;
    (*pcVar3)();
                    /* WARNING: Call to offcut address within same function */
    *(undefined2 *)(puVar15 + -2) = 0x1000;
    puVar16 = puVar15 + -4;
    *(undefined2 *)(puVar15 + -4) = 0xd8;
    func_0x000101d2();
    *(undefined2 *)(puVar16 + -2) = 0x1000;
    *(undefined2 *)(puVar16 + -4) = 0xdd;
    FUN_1000_02d0();
    pcVar4 = (code *)swi(0x1a);
    (*pcVar4)();
    *(undefined2 *)&DAT_2cff_0083 = extraout_DX_00;
    *(int *)&DAT_2cff_0085 = iVar9;
    *(undefined2 *)(puVar16 + -2) = 0x1000;
    pcVar3 = (code *)*(undefined2 *)&DAT_2cff_2a4a;
    puVar17 = puVar16 + -4;
    *(undefined2 *)(puVar16 + -4) = 0xee;
    (*pcVar3)();
    *(undefined2 *)(puVar17 + -2) = *(undefined2 *)&DAT_2cff_0073;
    *(undefined2 *)(puVar17 + -4) = *(undefined2 *)&DAT_2cff_0071;
    *(undefined2 *)(puVar17 + -6) = *(undefined2 *)&DAT_2cff_006f;
    *(undefined2 *)(puVar17 + -8) = *(undefined2 *)&DAT_2cff_006d;
    *(undefined2 *)(puVar17 + -10) = *(undefined2 *)&DAT_2cff_006b;
    *(undefined2 *)(puVar17 + -0xc) = 0x1000;
    *(undefined2 *)(puVar17 + -0xe) = 0x107;
    uVar24 = FUN_22ab_2ed8();
    *(undefined2 *)(puVar17 + -0xc) = uVar24;
    *(undefined2 *)(puVar17 + -0xe) = 0x22ab;
    *(undefined2 *)(puVar17 + -0x10) = 0x10d;
    FUN_2ae3_0008();
    *(undefined2 *)(puVar17 + -0xe) = 0x1000;
    *(undefined2 *)(puVar17 + -0x10) = 0x117;
    FUN_1000_0172();
    *(undefined2 *)(puVar17 + -0xe) = 0x1000;
    pcVar4 = pcRam00002a48;
    puVar18 = puVar17 + -0x10;
    *(undefined2 *)(puVar17 + -0x10) = 0x11c;
    (*pcVar4)();
    pcVar4 = (code *)swi(0x21);
    (*pcVar4)();
    cVar10 = '\0';
    *(undefined2 *)(puVar18 + -2) = 0x1be;
    FUN_1000_01a7();
    *(undefined2 *)(puVar18 + -2) = 3;
    *(undefined2 *)(puVar18 + -4) = 0x1000;
    *(undefined2 *)(puVar18 + -6) = 0x1c7;
    cVar6 = FUN_1000_010d();
    *(char *)(iVar12 + unaff_SI) = *(char *)(iVar12 + unaff_SI) + cVar6;
    *(char *)(iVar12 + unaff_SI) = *(char *)(iVar12 + unaff_SI) + cVar6;
    *(char *)(iVar12 + unaff_SI) = *(char *)(iVar12 + unaff_SI) + cVar6;
    *(char *)(iVar12 + unaff_SI) = *(char *)(iVar12 + unaff_SI) + cVar6;
    *(char *)(iVar12 + unaff_SI) = *(char *)(iVar12 + unaff_SI) + cVar6;
    cRam0000068f = cRam0000068f + cVar10;
    return;
  }
LAB_1000_0099:
  FUN_1000_01af();
  return;
}


