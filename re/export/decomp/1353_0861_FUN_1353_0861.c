// FUN_1353_0861 @ 1353:0861

void __cdecl16far
FUN_1353_0861(undefined2 param_1,undefined2 param_2,int param_3,undefined2 param_4,int param_5,
             undefined1 *param_6,uint param_7)

{
  bool bVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined1 local_8;
  undefined1 local_4;
  undefined1 local_3;
  
  bVar1 = true;
  if (param_5 == 1) {
    iVar5 = 8;
  }
  else if (param_5 == 2) {
    iVar5 = 6;
  }
  else {
    iVar5 = 0;
  }
  FUN_1353_074e(param_1,param_2,param_3,param_4,param_5,(undefined1 *)param_6,param_6._2_2_);
  local_3 = 0;
  do {
    iVar2 = FUN_2b9a_000b((undefined1 *)param_6,param_6._2_2_,param_4,param_5);
    iVar2 = FUN_1353_07f1(param_1,param_2,iVar2 * iVar5 + param_3);
    if ((iVar2 < 0x20) || (0x7f < iVar2)) {
      if (((iVar2 == 8) || (iVar2 == 0xcb)) &&
         (iVar4 = FUN_2b9a_000b((undefined1 *)param_6,param_6._2_2_), iVar4 != 0)) {
        iVar4 = FUN_2b9a_000b((undefined1 *)param_6,param_6._2_2_);
        ((undefined1 *)param_6)[iVar4 + -1] = 0;
      }
    }
    else {
      if (bVar1) {
        for (iVar4 = 0; ((undefined1 *)param_6)[iVar4] != '\0'; iVar4 = iVar4 + 1) {
          ((undefined1 *)param_6)[iVar4] = 0x20;
        }
        ((undefined1 *)param_6)[iVar4] = 0x20;
        ((undefined1 *)param_6)[iVar4 + 1] = 0;
        FUN_1353_074e(param_1,param_2,param_3,param_4,param_5,(undefined1 *)param_6,param_6._2_2_);
        *param_6 = 0;
      }
      uVar3 = FUN_2b9a_000b((undefined1 *)param_6,param_6._2_2_);
      if (uVar3 < param_7) {
        iVar4 = FUN_2b9a_000b((undefined1 *)param_6,param_6._2_2_);
        local_8 = (undefined1)iVar2;
        ((undefined1 *)param_6)[iVar4] = local_8;
        ((undefined1 *)param_6)[iVar4 + 1] = 0;
        local_4 = local_8;
        FUN_1353_074e(param_1,param_2,iVar5 * iVar4 + param_3,param_4,param_5,&local_4);
      }
    }
    bVar1 = false;
  } while ((iVar2 != 0xd) && (iVar2 != 0x1b));
  return;
}


