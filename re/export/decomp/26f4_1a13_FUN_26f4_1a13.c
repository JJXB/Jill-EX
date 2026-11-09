// FUN_26f4_1a13 @ 26f4:1a13

void __cdecl16far FUN_26f4_1a13(undefined2 param_1)

{
  if (DAT_2cff_83aa < 0xf) {
    DAT_2cff_be38 = DAT_2cff_be38 | 0xc000;
    ((undefined2 *)&DAT_2cff_83ac)[DAT_2cff_83aa] = param_1;
    DAT_2cff_83aa = DAT_2cff_83aa + 1;
  }
  return;
}


