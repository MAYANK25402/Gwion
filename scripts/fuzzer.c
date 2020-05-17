#include "gwion_util.h"
#include "gwion_ast.h"
#include "gwion_ast.h"
#include "gwion_env.h"
#include "vm.h"
#include "gwion.h"
#include "arg.h"
#include "compile.h"

static struct Gwion_ gwion;

static void initialize() {
  Arg arg = { .loop=-1 };
  const m_bool ini = gwion_ini(&gwion, &arg);
  arg_release(&arg);
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if(!gwion.mp)
    initialize();
  push_global(&gwion, "[afl]");
  m_str str = mp_calloc2(gwion.mp, Size + 1);
  memcpy(str, Data, Size);
  str[Size] = '\0';
  if(compile_string(&gwion, "libfuzzer", str))
    gwion_run(&gwion);
  pop_global(&gwion);
  mp_free2(gwion.mp, Size + 1, str);
  return 0;
}