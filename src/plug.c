#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include "defs.h"
#include "absyn.h"
#include "err_msg.h"
#include "operator.h"
#include "import.h"
#include "plug.h"
#ifdef JIT
#include "instr.h"
#include "jitter.h"
#endif
static inline int so_filter(const struct dirent* dir) {
  return strstr(dir->d_name, ".so") ? 1 : 0;
}

ANN static void handle_plug(PlugInfo v, const m_str c) {
  void* handler = dlopen(c, RTLD_LAZY);
  if(handler) {
    vector_add(&v[0], (vtype)handler);
    m_bool(*import)(Gwi) = (m_bool(*)(Gwi))(intptr_t)dlsym(handler, "import");
    if(import)
      vector_add(&v[1], (vtype)import);
#ifdef JIT
    void (*jit_ctrl_import)(struct Jit*) = (void (*)(struct Jit*))(intptr_t)dlsym(handler, "_jit_ctrl_import");
    if(jit_ctrl_import)
      vector_add(&v[2], (vtype)jit_ctrl_import);
    void (*jit_code_import)(struct Jit*) = (void (*)(struct Jit*))(intptr_t)dlsym(handler, "_jit_code_import");
    if(jit_code_import)
      vector_add(&v[3], (vtype)jit_code_import);
#endif
  } else
    err_msg(TYPE_, 0, "error in %s.", dlerror());
}

void plug_ini(PlugInfo v, Vector list) {
  vector_init(&v[0]);
  vector_init(&v[1]);
#ifdef JIT
  vector_init(&v[2]);
  vector_init(&v[3]);
  vector_init(&v[4]);
#endif
  for(m_uint i = 0; i < vector_size(list); i++) {
   const m_str dirname = (m_str)vector_at(list, i);
   struct dirent **namelist;
   int n = scandir(dirname, &namelist, so_filter, alphasort);
   if(n > 0) {
     while(n--) {
       char c[strlen(dirname) + strlen(namelist[n]->d_name) + 2];
       sprintf(c, "%s/%s", dirname, namelist[n]->d_name);
       handle_plug(v, c);
       free(namelist[n]);
      }
     free(namelist);
    }
  }
}

void plug_end(PlugInfo v) {
  for(m_uint i = 0; i < vector_size(&v[0]); ++i)
    dlclose((void*)vector_at(&v[0], i));
  vector_release(&v[0]);
  vector_release(&v[1]);
#ifdef JIT
  vector_release(&v[2]);
  vector_release(&v[3]);
  vector_release(&v[4]);
#endif
}
