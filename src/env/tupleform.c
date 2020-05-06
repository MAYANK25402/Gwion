#include "gwion_util.h"
#include "gwion_ast.h"
#include "gwion_env.h"
#include "vm.h"
#include "instr.h"
#include "object.h"
#include "emit.h"
#include "vm.h"
#include "gwion.h"
#include "operator.h"
#include "import.h"
#include "gwi.h"
#include "traverse.h"
#include "object.h"
#include "parse.h"
#include "array.h"

ANN void tuple_info(const Env env, const Value v) {
  const m_uint offset = vector_back(&env->class_def->e->tuple->offset);
  vector_add(&env->class_def->e->tuple->types, (vtype)v->type);
  vector_add(&env->class_def->e->tuple->offset, offset + v->type->size);
}

ANN void tuple_contains(const Env env, const Value value) {
  if(!env->class_def->e->tuple)
    return;
  const Type t = value->type;
  const Vector v = &env->class_def->e->tuple->contains;
  const m_int idx = vector_size(v) ? vector_find(v, (vtype)t) : -1;
  if(idx == -1) {
    ADD_REF(t);
    vector_add(v, (vtype)t);
  }
}

ANN2(1) TupleForm new_tupleform(MemPool p, const Type parent_type) {
  TupleForm tuple = mp_malloc(p, TupleForm);
  vector_init(&tuple->contains);
  vector_init(&tuple->types);
  vector_init(&tuple->offset);
  if(parent_type && parent_type->e->tuple) {
    const TupleForm parent = parent_type->e->tuple;
    const m_uint sz = vector_size(&parent->types);
    tuple->start = parent->start + sz;
    if(sz) {
      const Type last = (Type)vector_back(&parent->types);
      const m_uint offset = vector_back(&parent->offset);
      vector_add(&tuple->offset, offset + last->size);
    } else {
      vector_add(&tuple->offset, 0);
    }
  } else {
    vector_add(&tuple->offset, 0);
    tuple->start = 0;
  }
  return tuple;
}

ANN void free_tupleform(const TupleForm tuple, const struct Gwion_ *gwion) {
  for(m_uint i = 0; i < vector_size(&tuple->contains); ++i)
    REM_REF((Type)vector_at(&tuple->contains, i), (void*)gwion);
  vector_release(&tuple->contains);
  vector_release(&tuple->types);
  vector_release(&tuple->offset);
}