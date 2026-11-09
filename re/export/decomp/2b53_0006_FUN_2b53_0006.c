// FUN_2b53_0006 @ 2b53:0006

void __cdecl16far FUN_2b53_0006(int param_1)

{
  if ((param_1 < 0) || (0x13 < param_1)) {
    FUN_2adf_000c(6);
  }
  else {
    *(undefined2 *)(param_1 * 2 + 0x2954) = 0xffff;
    FUN_2b56_0007(param_1);
  }
  return;
}


