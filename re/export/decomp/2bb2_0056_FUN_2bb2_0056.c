// FUN_2bb2_0056 @ 2bb2:0056

void __cdecl16near FUN_2bb2_0056(void)

{
  int unaff_BP;
  int unaff_DI;
  int iVar1;
  undefined2 unaff_SS;
  
  iVar1 = unaff_DI - (unaff_BP + -0x52);
  (*(code *)*(undefined2 *)(unaff_BP + 0x12))
            (0x2bb2,*(undefined2 *)(unaff_BP + 0xe),*(undefined2 *)(unaff_BP + 0x10),iVar1,
             unaff_BP + -0x52);
  *(undefined1 *)(unaff_BP + -0x53) = 0x50;
  *(int *)(unaff_BP + -0x56) = *(int *)(unaff_BP + -0x56) + iVar1;
  return;
}


