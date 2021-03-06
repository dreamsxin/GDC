/* d-tree.h -- Definitions and declarations for code generation.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef GCC_D_TREE_H
#define GCC_D_TREE_H

/* Forward type declarations to avoid including unnecessary headers.  */

class Dsymbol;
class Declaration;
class AggregateDeclaration;
class ClassDeclaration;
class EnumDeclaration;
class FuncDeclaration;
class TypeInfoDeclaration;
class VarDeclaration;
class Expression;
class ClassReferenceExp;
class Module;
class Statement;
class Type;
class TypeFunction;

/* Usage of TREE_LANG_FLAG_?:
   0: METHOD_CALL_EXPR

   Usage of TYPE_LANG_FLAG_?:
   0: TYPE_SHARED
   1: TYPE_IMAGINARY_FLOAT (in REAL_TYPE).
      ANON_AGGR_TYPE_P (in RECORD_TYPE, UNION_TYPE).
   2: CLASS_TYPE_P (in RECORD_TYPE).

   Usage of DECL_LANG_FLAG_?:
   0: D_DECL_ONE_ONLY
   1: D_DECL_IS_TEMPLATE
   2: LABEL_VARIABLE_CASE (in LABEL_DECL).  */

/* The kinds of scopes we recognise.  */

enum level_kind
{
  level_block,		/* An ordinary block scope.  */
  level_try,		/* A try-block.  */
  level_catch,		/* A catch-block.  */
  level_finally,	/* A finally-block.  */
  level_cond,		/* The scope for an if condition.  */
  level_switch,		/* The scope for a switch statement.  */
  level_loop,		/* A for, do-while, or unrolled-loop block.  */
  level_with,		/* The scope for a with statement.  */
  level_function,	/* The block representing an entire function.  */
};

/* For use with break and continue statements.  */

enum bc_kind
{
  bc_break    = 0,
  bc_continue = 1,
};

/* The datatype used to implement D scope.  It is needed primarily to support
   the backend, but also helps debugging information for local variables.  */

struct GTY((chain_next ("%h.level_chain"))) binding_level
{
  /* A chain of declarations for all variables, constants and functions.
     These are in the reverse of the order supplied.  */
  tree names;

  /* For each level (except the global one), a chain of BLOCK nodes for
     all the levels that were entered and exited one level down.  */
  tree blocks;

  /* The binding level this one is contained in.  */
  binding_level *level_chain;

  /* The kind of scope this object represents.  */
  ENUM_BITFIELD (level_kind) kind : 4;
};

/* The binding level currently in effect.  */
extern GTY(()) binding_level *current_binding_level;
extern GTY(()) binding_level *global_binding_level;

/* Used only for jumps to as-yet undefined labels, since jumps to
   defined labels can have their validity checked immediately.  */

struct GTY((chain_next ("%h.next"))) d_label_use_entry
{
  d_label_use_entry *next;

  /* The frontend Statement associated with the jump.  */
  Statement * GTY((skip)) statement;

  /* The binding level to which this entry is *currently* attached.
     This is initially the binding level in which the goto appeared,
     but is modified as scopes are closed.  */
  binding_level *level;
};

/* A list of all LABEL_DECLs in the function that have names.  Here so
   we can clear out their names' definitions at the end of the
   function, and so we can check the validity of jumps to these labels.  */

struct GTY(()) d_label_entry
{
  /* The label decl itself.  */
  tree label;

  /* The frontend Statement associated with the label.  */
  Statement * GTY((skip)) statement;

  /* The binding level to which the label is *currently* attached.
     This is initially set to the binding level in which the label
     is defined, but is modified as scopes are closed.  */
  binding_level *level;

  /* A list of forward references of the label.  */
  d_label_use_entry *fwdrefs;

  /* The following bits are set after the label is defined, and are
     updated as scopes are popped.  They indicate that a backward jump
     to the label will illegally enter a scope of the given flavor.  */
  bool in_try_scope;
  bool in_catch_scope;

  /* If set, the label we reference represents a break/continue pair.  */
  bool bc_label;
};

/* Frame information for a function declaration.  */

struct GTY(()) tree_frame_info
{
    struct tree_common common;
    tree frame_type;
};

/* True if the function creates a nested frame.  */
#define FRAMEINFO_CREATES_FRAME(NODE) \
  (TREE_LANG_FLAG_0 (FUNCFRAME_INFO_CHECK (NODE)))

/* True if the function has a static chain passed in it's DECL_ARGUMENTS.  */
#define FRAMEINFO_STATIC_CHAIN(NODE) \
  (TREE_LANG_FLAG_1 (FUNCFRAME_INFO_CHECK (NODE)))

/* True if the function frame is a closure (initialized on the heap).  */
#define FRAMEINFO_IS_CLOSURE(NODE) \
  (TREE_LANG_FLAG_2 (FUNCFRAME_INFO_CHECK (NODE)))

#define FRAMEINFO_TYPE(NODE) \
  (((tree_frame_info *) FUNCFRAME_INFO_CHECK (NODE))->frame_type)

/* Language-dependent contents of an identifier.  */

struct GTY(()) lang_identifier
{
  struct tree_identifier common;

  /* The identifier as the user sees it.  */
  tree pretty_ident;

  /* The frontend Declaration associated with this identifier.  */
  Declaration * GTY((skip)) dsymbol;
};

#define IDENTIFIER_LANG_SPECIFIC(NODE) \
  ((struct lang_identifier *) IDENTIFIER_NODE_CHECK (NODE))

#define IDENTIFIER_PRETTY_NAME(NODE) \
  (IDENTIFIER_LANG_SPECIFIC (NODE)->pretty_ident)

#define IDENTIFIER_DSYMBOL(NODE) \
  (IDENTIFIER_LANG_SPECIFIC (NODE)->dsymbol)

/* Global state pertinent to the current function.  */

struct GTY(()) language_function
{
  /* Our function and enclosing module.  */
  FuncDeclaration * GTY((skip)) function;
  Module * GTY((skip)) module;

  /* Static chain of function, for D2, this is a closure.  */
  tree static_chain;

  /* Stack of statement lists being collected while we are
     compiling the function.  */
  vec<tree, va_gc> *stmt_list;

  /* Any nested functions that were deferred during codegen.  */
  vec<FuncDeclaration *> GTY((skip)) deferred_fns;

  /* Variables that are in scope that will need destruction later.  */
  vec<VarDeclaration *> GTY((skip)) vars_in_scope;

  /* Table of all used or defined labels in the function.  */
  hash_map<Statement *, d_label_entry> *labels;
};

/* The D front end types have not been integrated into the GCC garbage
   collection system.  Handle this by using the "skip" attribute.  */

struct GTY(()) lang_decl
{
  Declaration * GTY((skip)) decl;

  tree initial;

  /* FIELD_DECL in frame struct that this variable is allocated in.  */
  tree frame_field;

  /* RESULT_DECL in a function that returns by nrvo.  */
  tree named_result;

  /* Chain of DECL_LANG_THUNKS in a function.  */
  tree thunks;

  /* In a FUNCTION_DECL, this is the THUNK_LANG_OFFSET.  */
  int offset;

  /* FUNCFRAME_INFO in a function that has non-local references.  */
  tree frame_info;
};

/* The D frontend Declaration AST for GCC decl NODE.  */
#define DECL_LANG_FRONTEND(NODE) \
  (DECL_LANG_SPECIFIC (NODE) \
   ? DECL_LANG_SPECIFIC (NODE)->decl : NULL)

#define DECL_LANG_INITIAL(NODE) \
  DECL_LANG_SPECIFIC (NODE)->initial

#define SET_DECL_LANG_FRAME_FIELD(NODE, VAL) \
  DECL_LANG_SPECIFIC (NODE)->frame_field = VAL

#define DECL_LANG_FRAME_FIELD(NODE) \
  (DECL_P (NODE) \
   ? DECL_LANG_SPECIFIC (NODE)->frame_field : NULL)

#define SET_DECL_LANG_NRVO(NODE, VAL) \
  DECL_LANG_SPECIFIC (NODE)->named_result = VAL

#define DECL_LANG_NRVO(NODE) \
  (DECL_P (NODE) \
   ? DECL_LANG_SPECIFIC (NODE)->named_result : NULL)

#define DECL_LANG_THUNKS(NODE) \
  DECL_LANG_SPECIFIC (NODE)->thunks

#define THUNK_LANG_OFFSET(NODE) \
  DECL_LANG_SPECIFIC (NODE)->offset

#define DECL_LANG_FRAMEINFO(NODE) \
  DECL_LANG_SPECIFIC (NODE)->frame_info

/* The lang_type field is not set for every GCC type.  */

struct GTY(()) lang_type
{
  Type * GTY((skip)) type;
};

/* The D frontend Type AST for GCC type NODE.  */
#define TYPE_LANG_FRONTEND(NODE) \
  (TYPE_LANG_SPECIFIC (NODE) \
   ? TYPE_LANG_SPECIFIC (NODE)->type : NULL)


enum d_tree_node_structure_enum
{
  TS_D_GENERIC,
  TS_D_IDENTIFIER,
  TS_D_FRAMEINFO,
  LAST_TS_D_ENUM
};

/* The resulting tree type.  */

union GTY((desc ("d_tree_node_structure (&%h)"),
	   chain_next ("CODE_CONTAINS_STRUCT (TREE_CODE (&%h.generic), TS_COMMON)"
		       " ? ((union lang_tree_node *) TREE_CHAIN (&%h.generic)) : NULL")))
lang_tree_node
{
  union tree_node GTY ((tag ("TS_D_GENERIC"),
			desc ("tree_node_structure (&%h)"))) generic;
  lang_identifier GTY ((tag ("TS_D_IDENTIFIER"))) identifier;
  tree_frame_info GTY ((tag ("TS_D_FRAMEINFO"))) frameinfo;
};

/* True if the Tdelegate typed expression is not really a variable,
   but a literal function / method reference.  */
#define METHOD_CALL_EXPR(NODE) \
  (TREE_LANG_FLAG_0 (NODE))

/* True if the type was declared 'shared'.  */
#define TYPE_SHARED(NODE) \
  (TYPE_LANG_FLAG_0 (NODE))

/* True if the type is an imaginary float type.  */
#define TYPE_IMAGINARY_FLOAT(NODE) \
  (TYPE_LANG_FLAG_1 (REAL_TYPE_CHECK (NODE)))

/* True if the type is an anonymous record or union.  */
#define ANON_AGGR_TYPE_P(NODE) \
  (TYPE_LANG_FLAG_1 (RECORD_OR_UNION_CHECK (NODE)))

/* True if the type is the underlying record for a class.  */
#define CLASS_TYPE_P(NODE) \
  (TYPE_LANG_FLAG_2 (RECORD_TYPE_CHECK (NODE)))

/* True if the symbol should be made "link one only".  This is used to
   defer calling make_decl_one_only() before the decl has been prepared.  */
#define D_DECL_ONE_ONLY(NODE) \
  (DECL_LANG_FLAG_0 (NODE))

/* True if the symbol is a template member.  Need to distinguish between
   templates and other shared static data so that the latter is not affected
   by -femit-templates.  */
#define D_DECL_IS_TEMPLATE(NODE) \
  (DECL_LANG_FLAG_1 (NODE))

/* True if the decl is a variable case label decl.  */
#define LABEL_VARIABLE_CASE(NODE) \
  (DECL_LANG_FLAG_2 (LABEL_DECL_CHECK (NODE)))

enum d_tree_index
{
  DTI_VTBL_PTR_TYPE,

  DTI_BOOL_TYPE,
  DTI_CHAR_TYPE,
  DTI_WCHAR_TYPE,
  DTI_DCHAR_TYPE,

  DTI_BYTE_TYPE,
  DTI_UBYTE_TYPE,
  DTI_SHORT_TYPE,
  DTI_USHORT_TYPE,
  DTI_INT_TYPE,
  DTI_UINT_TYPE,
  DTI_LONG_TYPE,
  DTI_ULONG_TYPE,
  DTI_CENT_TYPE,
  DTI_UCENT_TYPE,

  DTI_IFLOAT_TYPE,
  DTI_IDOUBLE_TYPE,
  DTI_IREAL_TYPE,

  DTI_UNKNOWN_TYPE,

  DTI_MAX
};

extern GTY(()) tree d_global_trees[DTI_MAX];

#define vtbl_ptr_type_node		d_global_trees[DTI_VTBL_PTR_TYPE]
#define bool_type_node			d_global_trees[DTI_BOOL_TYPE]
#define char8_type_node			d_global_trees[DTI_CHAR_TYPE]
#define char16_type_node		d_global_trees[DTI_DCHAR_TYPE]
#define char32_type_node		d_global_trees[DTI_WCHAR_TYPE]
#define byte_type_node			d_global_trees[DTI_BYTE_TYPE]
#define ubyte_type_node			d_global_trees[DTI_UBYTE_TYPE]
#define short_type_node			d_global_trees[DTI_SHORT_TYPE]
#define ushort_type_node		d_global_trees[DTI_USHORT_TYPE]
#define int_type_node			d_global_trees[DTI_INT_TYPE]
#define uint_type_node			d_global_trees[DTI_UINT_TYPE]
#define long_type_node			d_global_trees[DTI_LONG_TYPE]
#define ulong_type_node			d_global_trees[DTI_ULONG_TYPE]
#define cent_type_node			d_global_trees[DTI_CENT_TYPE]
#define ucent_type_node			d_global_trees[DTI_UCENT_TYPE]
#define ifloat_type_node		d_global_trees[DTI_IFLOAT_TYPE]
#define idouble_type_node		d_global_trees[DTI_IDOUBLE_TYPE]
#define ireal_type_node			d_global_trees[DTI_IREAL_TYPE]
#define unknown_type_node		d_global_trees[DTI_UNKNOWN_TYPE]

/* In d-builtins.cc.  */
extern const attribute_spec d_langhook_attribute_table[];
extern const attribute_spec d_langhook_common_attribute_table[];
extern const attribute_spec d_langhook_format_attribute_table[];

extern tree d_builtin_function (tree);
extern void d_init_builtins (void);
extern void d_register_builtin_type (tree, const char *);
extern void d_build_builtins_module (Module *);
extern void d_maybe_set_builtin (Module *);
extern Expression *build_expression (tree);

/* In d-convert.cc.  */
extern tree d_truthvalue_conversion (tree);

/* In d-decls.cc.  */
extern tree make_internal_name (Dsymbol *, const char *, const char *);
extern tree get_symbol_decl (Declaration *);
extern tree make_thunk (FuncDeclaration *, int);
extern tree layout_moduleinfo_fields (Module *, tree);
extern tree get_moduleinfo_decl (Module *);
extern tree get_typeinfo_decl (TypeInfoDeclaration *);
extern tree get_classinfo_decl (ClassDeclaration *);
extern tree get_vtable_decl (ClassDeclaration *);
extern tree build_new_class_expr (ClassReferenceExp *expr);
extern tree aggregate_initializer (AggregateDeclaration *);
extern tree enum_initializer (EnumDeclaration *);

/* In d-expr.cc.  */
extern tree build_expr (Expression *, bool = false);
extern tree build_expr_dtor (Expression *);
extern tree build_return_dtor (Expression *, Type *, TypeFunction *);

/* In d-incpath.cc.  */
extern void add_import_paths (const char *, const char *, bool);

/* In d-lang.cc.  */
extern d_tree_node_structure_enum d_tree_node_structure (lang_tree_node *);
extern struct lang_type *build_lang_type (Type *);
extern struct lang_decl *build_lang_decl (Declaration *);
extern tree d_pushdecl (tree);
extern tree d_unsigned_type (tree);
extern tree d_signed_type (tree);
extern void d_keep (tree);

/* In imports.cc.  */
extern tree build_import_decl (Dsymbol *);

/* In typeinfo.cc.  */
extern tree build_typeinfo (Type *);
extern tree layout_typeinfo (TypeInfoDeclaration *);

/* In toir.cc.  */
extern void build_ir (FuncDeclaration *);

/* In types.cc.  */
extern tree build_ctype (Type *);

#endif
