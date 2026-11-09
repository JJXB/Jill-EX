// FUN_22ab_0dc6 @ 22ab:0dc6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __cdecl16far FUN_22ab_0dc6(int param_1,undefined4 param_2,int param_3)

{
  char cVar1;
  undefined1 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined2 uVar6;
  int local_6;
  
  local_6 = 7;
  iVar4 = 0;
  iVar5 = 0;
  while( true ) {
    iVar3 = (int)_DAT_2cff_3f0a;
    uVar6 = (undefined2)((ulong)_DAT_2cff_3f0a >> 0x10);
    if ((param_1 <= iVar5) || (DAT_2cff_439a <= iVar4)) break;
    iVar5 = iVar5 + (uint)(*(char *)(iVar3 + iVar4) == '\r');
    iVar4 = iVar4 + 1;
  }
  for (; (*(char *)(iVar3 + iVar4) < ' ' && (*(char *)(iVar3 + iVar4) != '\r')); iVar4 = iVar4 + 1)
  {
  }
  if (('/' < *(char *)(iVar3 + iVar4)) && (*(char *)(iVar3 + iVar4) < '8')) {
    local_6 = *(char *)(iVar3 + iVar4) + -0x30;
    iVar4 = iVar4 + 1;
  }
  iVar5 = 0;
  while( true ) {
    cVar1 = *(char *)((int)_DAT_2cff_3f0a + iVar4);
    iVar3 = (int)param_2;
    uVar6 = (undefined2)((ulong)param_2 >> 0x10);
    if (((cVar1 == '\r') || (DAT_2cff_439a <= iVar4)) || (0x4c < iVar5)) break;
    if (param_3 == 0) {
      *(char *)(iVar3 + iVar5) = cVar1;
    }
    else {
      uVar2 = FUN_2ce7_000d((int)cVar1);
      *(undefined1 *)(iVar3 + iVar5) = uVar2;
    }
    iVar5 = iVar5 + 1;
    iVar4 = iVar4 + 1;
  }
  *(undefined1 *)(iVar3 + iVar5) = 0;
  return local_6;
}


