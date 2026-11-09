// FUN_15ed_01a8 @ 15ed:01a8

void __cdecl16near FUN_15ed_01a8(void)

{
  char in_AL;
  undefined2 *puVar1;
  undefined2 in_DX;
  undefined2 *in_BX;
  
  puVar1 = (undefined2 *)CONCAT11((char)(in_AL + '\b') >> 7,(in_AL + '\b') * '\x04');
  *in_BX = *puVar1;
  *puVar1 = in_DX;
  in_BX[1] = puVar1[1];
  puVar1[1] = (char *)s_Run_this_game_without_any_TSR_s_i_2cff_15d3 + 0x1a;
  return;
}


