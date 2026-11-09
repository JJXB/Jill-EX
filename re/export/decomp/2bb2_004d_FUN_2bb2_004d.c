// FUN_2bb2_004d @ 2bb2:004d

void __cdecl16near FUN_2bb2_004d(void)

{
  char *pcVar1;
  char cVar2;
  undefined1 in_AL;
  int unaff_BP;
  undefined1 *unaff_DI;
  undefined2 unaff_SS;
  
  *unaff_DI = in_AL;
  pcVar1 = (char *)(unaff_BP + -0x53);
  cVar2 = *pcVar1;
  *pcVar1 = *pcVar1 + -1;
  if (*pcVar1 != '\0' && SBORROW1(cVar2,'\x01') == *pcVar1 < '\0') {
    (*(code *)*(undefined2 *)(unaff_BP + 0x12))
              (0x2bb2,*(undefined2 *)(unaff_BP + 0xe),*(undefined2 *)(unaff_BP + 0x10),
               unaff_DI + (1 - (unaff_BP + -0x52)),unaff_BP + -0x52);
    *(undefined1 *)(unaff_BP + -0x53) = 0x50;
    *(int *)(unaff_BP + -0x56) =
         (int)(unaff_DI + (1 - (unaff_BP + -0x52)) + *(int *)(unaff_BP + -0x56));
  }
  return;
}


