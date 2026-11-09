// FUN_2b4b_000c @ 2b4b:000c

int __cdecl16far FUN_2b4b_000c(undefined2 param_1,undefined2 param_2,uint param_3)

{
  code *pcVar1;
  int iVar2;
  undefined2 unaff_BP;
  bool bVar3;
  
  bVar3 = false;
  pcVar1 = (code *)swi(0x21);
  iVar2 = (*pcVar1)();
  if (bVar3) {
    iVar2 = FUN_2adf_000c(iVar2);
  }
  else {
    *(uint *)(iVar2 * 2 + 0x2954) = param_3 & 0xf8ff | 0x8000;
  }
  return iVar2;
}


