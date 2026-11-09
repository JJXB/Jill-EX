// FUN_14c4_0009 @ 14c4:0009

undefined2 __cdecl16far FUN_14c4_0009(byte *param_1,int *param_2,int param_3)

{
  byte *pbVar1;
  byte bVar2;
  int *piVar3;
  undefined2 in_AX;
  int iVar4;
  byte bVar6;
  uint uVar5;
  int iVar7;
  byte *pbVar8;
  byte *pbVar9;
  int *piVar10;
  int *piVar11;
  undefined2 uVar12;
  undefined2 uVar13;
  bool bVar14;
  
  uVar13 = (undefined2)((ulong)param_1 >> 0x10);
  uVar12 = (undefined2)((ulong)param_2 >> 0x10);
  piVar10 = (int *)param_2;
  if (param_3 != 0) {
    iVar4 = 0;
    pbVar8 = (byte *)param_1;
    piVar11 = piVar10;
    do {
      while( true ) {
        bVar2 = *pbVar8;
        bVar6 = (byte)((uint)iVar4 >> 8);
        iVar4 = CONCAT11(bVar6,bVar2);
        if (bVar2 < 0x20) break;
        *piVar11 = iVar4;
        piVar11 = piVar11 + 1;
LAB_14c4_002a:
        param_3 = param_3 + -1;
        pbVar8 = pbVar8 + 1;
        if (param_3 == 0) {
          return in_AX;
        }
      }
      if (bVar2 < 0x10) {
        uVar5 = CONCAT11(bVar6,bVar2) & 0xf0ff;
        iVar4 = (uint)(byte)((byte)(uVar5 >> 8) | (byte)uVar5) << 8;
        goto LAB_14c4_002a;
      }
      if (bVar2 == 0x18) {
        piVar10 = piVar10 + 0x50;
        piVar11 = piVar10;
        goto LAB_14c4_002a;
      }
      if (bVar2 < 0x18) {
        uVar5 = CONCAT11(bVar6,bVar2 * '\x10') & 0x8fff;
        iVar4 = (uint)(byte)((byte)(uVar5 >> 8) | (byte)uVar5) << 8;
        goto LAB_14c4_002a;
      }
      if (0x1a < bVar2) {
        if (bVar2 == 0x1b) {
          iVar4 = (uint)(bVar6 ^ 0x80) << 8;
        }
        goto LAB_14c4_002a;
      }
      pbVar9 = pbVar8 + 2;
      iVar4 = CONCAT11(bVar6,0x20);
      if (bVar2 != 0x19) {
        pbVar1 = pbVar9;
        pbVar9 = pbVar8 + 3;
        iVar4 = CONCAT11(bVar6,*pbVar1);
        param_3 = param_3 + -1;
      }
      for (iVar7 = pbVar8[1] + 1; iVar7 != 0; iVar7 = iVar7 + -1) {
        piVar3 = piVar11;
        piVar11 = piVar11 + 1;
        *piVar3 = iVar4;
      }
      bVar14 = param_3 != 1;
      param_3 = param_3 + -2;
      pbVar8 = pbVar9;
    } while (param_3 != 0 && bVar14);
  }
  return in_AX;
}


