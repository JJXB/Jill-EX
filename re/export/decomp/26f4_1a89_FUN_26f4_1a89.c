// FUN_26f4_1a89 @ 26f4:1a89

int __cdecl16far FUN_26f4_1a89(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 0;
  for (iVar1 = 0; iVar1 < DAT_2cff_83aa; iVar1 = iVar1 + 1) {
    iVar2 = iVar2 + (uint)(((undefined2 *)&DAT_2cff_83ac)[iVar1] == param_1);
  }
  return iVar2;
}


