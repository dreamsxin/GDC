/* GDC -- D front-end for GCC
   Copyright (C) 2011, 2012 Free Software Foundation, Inc.

   GCC is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   GCC is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.
*/

/* ARM unwind interface declarations for D.  This must match unwind-arm.h.  */

module gcc.unwind.arm;

import gcc.config;
static if (GNU_ARM_EABI_Unwinder):

import gcc.builtins;
import gcc.unwind.pe;

extern (C):

alias _Unwind_Word = __builtin_machine_uint;
alias _Unwind_Sword = __builtin_machine_int;
alias _Unwind_Ptr = __builtin_pointer_uint;
alias _Unwind_Internal_Ptr =__builtin_pointer_uint;
alias _uw = _Unwind_Word;
alias _uw64 = ulong;
alias _uw16 = ushort;
alias _uw8 = ubyte;

alias _Unwind_Reason_Code = uint;
enum : _Unwind_Reason_Code
{
  _URC_OK = 0,       /* operation completed successfully */
  _URC_FOREIGN_EXCEPTION_CAUGHT = 1,
  _URC_END_OF_STACK = 5,
  _URC_HANDLER_FOUND = 6,
  _URC_INSTALL_CONTEXT = 7,
  _URC_CONTINUE_UNWIND = 8,
  _URC_FAILURE = 9   /* unspecified failure of some kind */
}

alias _Unwind_State = int;
enum : _Unwind_State
{
  _US_VIRTUAL_UNWIND_FRAME = 0,
  _US_UNWIND_FRAME_STARTING = 1,
  _US_UNWIND_FRAME_RESUME = 2,
  _US_ACTION_MASK = 3,
  _US_FORCE_UNWIND = 8,
  _US_END_OF_STACK = 16
}

/* Provided only for for compatibility with existing code.  */
alias _Unwind_Action = int;
enum : _Unwind_Action
{
  _UA_SEARCH_PHASE =  1,
  _UA_CLEANUP_PHASE = 2,
  _UA_HANDLER_FRAME = 4,
  _UA_FORCE_UNWIND =  8,
  _UA_END_OF_STACK =  16,
  _URC_NO_REASON =    _URC_OK
}

struct _Unwind_Context;
alias _Unwind_EHT_Header = _uw;

extern(C) alias _Unwind_Exception_Cleanup_Fn
    = void function(_Unwind_Reason_Code, _Unwind_Exception *);

/* UCB: */

struct _Unwind_Control_Block
{
  _Unwind_Exception_Class exception_class = '\0';
  _Unwind_Exception_Cleanup_Fn exception_cleanup;
  /* Unwinder cache, private fields for the unwinder's use */
  struct _unwinder_cache
  {
    _uw reserved1;  /* Forced unwind stop fn, 0 if not forced */
    _uw reserved2;  /* Personality routine address */
    _uw reserved3;  /* Saved callsite address */
    _uw reserved4;  /* Forced unwind stop arg */
    _uw reserved5;
  }
  _unwinder_cache unwinder_cache;
  /* Propagation barrier cache (valid after phase 1): */
  struct _barrier_cache
  {
    _uw sp;
    _uw[5] bitpattern;
  }
  _barrier_cache barrier_cache;
  /* Cleanup cache (preserved over cleanup): */
  struct _cleanup_cache
  {
    _uw[4] bitpattern;
  }
  _cleanup_cache cleanup_cache;
  /* Pr cache (for pr's benefit): */
  struct _pr_cache
  {
    _uw fnstart;			/* function start address */
    _Unwind_EHT_Header *ehtp;		/* pointer to EHT entry header word */
    _uw additional;			/* additional data */
    _uw reserved1;
  }
  _pr_cache pr_cache;
  long[0] _force_alignment;     /* Force alignment to 8-byte boundary */
}

/* Virtual Register Set*/
alias _Unwind_VRS_RegClass = int;
enum : _Unwind_VRS_RegClass
{
  _UVRSC_CORE = 0,      /* integer register */
  _UVRSC_VFP = 1,       /* vfp */
  _UVRSC_FPA = 2,       /* fpa */
  _UVRSC_WMMXD = 3,     /* Intel WMMX data register */
  _UVRSC_WMMXC = 4      /* Intel WMMX control register */
}

alias _Unwind_VRS_DataRepresentation = int;
enum : _Unwind_VRS_DataRepresentation
{
  _UVRSD_UINT32 = 0,
  _UVRSD_VFPX = 1,
  _UVRSD_FPAX = 2,
  _UVRSD_UINT64 = 3,
  _UVRSD_FLOAT = 4,
  _UVRSD_DOUBLE = 5
}

alias _Unwind_VRS_Result = int;
enum : _Unwind_VRS_Result
{
  _UVRSR_OK = 0,
  _UVRSR_NOT_IMPLEMENTED = 1,
  _UVRSR_FAILED = 2
}

/* Frame unwinding state.  */
struct __gnu_unwind_state
{
  /* The current word (bytes packed msb first).  */
  _uw data;
  /* Pointer to the next word of data.  */
  _uw *next;
  /* The number of bytes left in this word.  */
  _uw8 bytes_left;
  /* The number of words pointed to by ptr.  */
  _uw8 words_left;
}

extern(C) alias personality_routine
    = _Unwind_Reason_Code function(_Unwind_State,
				   _Unwind_Control_Block *,
				   _Unwind_Context *);

_Unwind_VRS_Result _Unwind_VRS_Set(_Unwind_Context *, _Unwind_VRS_RegClass,
				   _uw, _Unwind_VRS_DataRepresentation,
				   void *);

_Unwind_VRS_Result _Unwind_VRS_Get(_Unwind_Context *, _Unwind_VRS_RegClass,
				   _uw, _Unwind_VRS_DataRepresentation,
				   void *);

_Unwind_VRS_Result _Unwind_VRS_Pop(_Unwind_Context *, _Unwind_VRS_RegClass,
				   _uw, _Unwind_VRS_DataRepresentation);


/* Support functions for the PR.  */
alias _Unwind_Exception = _Unwind_Control_Block;
alias _Unwind_Exception_Class = char[8];

void * _Unwind_GetLanguageSpecificData (_Unwind_Context *);
_Unwind_Ptr _Unwind_GetRegionStart (_Unwind_Context *);

_Unwind_Ptr _Unwind_GetDataRelBase (_Unwind_Context *);
/* This should never be used.  */
_Unwind_Ptr _Unwind_GetTextRelBase (_Unwind_Context *);

/* Interface functions: */
_Unwind_Reason_Code _Unwind_RaiseException(_Unwind_Control_Block *ucbp);
/*pragma (noreturn)*/ void _Unwind_Resume(_Unwind_Control_Block *ucbp);
_Unwind_Reason_Code _Unwind_Resume_or_Rethrow (_Unwind_Control_Block *ucbp);

extern(C) alias _Unwind_Stop_Fn
    =_Unwind_Reason_Code function(int, _Unwind_Action,
				  _Unwind_Exception_Class,
				  _Unwind_Control_Block *,
				  _Unwind_Context *, void *);

_Unwind_Reason_Code _Unwind_ForcedUnwind (_Unwind_Control_Block *,
					  _Unwind_Stop_Fn, void *);

/* @@@ Use unwind data to perform a stack backtrace.  The trace callback
   is called for every stack frame in the call chain, but no cleanup
   actions are performed.  */
extern(C) alias _Unwind_Trace_Fn
    = _Unwind_Reason_Code function(_Unwind_Context *, void *);

_Unwind_Reason_Code _Unwind_Backtrace(_Unwind_Trace_Fn, void *);

_Unwind_Word _Unwind_GetCFA (_Unwind_Context *);
void _Unwind_Complete(_Unwind_Control_Block *ucbp);
void _Unwind_DeleteException (_Unwind_Exception *);

_Unwind_Reason_Code __gnu_unwind_frame (_Unwind_Control_Block *,
					_Unwind_Context *);
_Unwind_Reason_Code __gnu_unwind_execute (_Unwind_Context *,
					  __gnu_unwind_state *);

_Unwind_Word
_Unwind_GetIPInfo (_Unwind_Context *context, int *ip_before_insn)
{
  *ip_before_insn = 0;
  return _Unwind_GetIP (context);
}

_Unwind_Word
_Unwind_GetGR (_Unwind_Context *context, int regno)
{
  _uw val;
  _Unwind_VRS_Get (context, _UVRSC_CORE, regno, _UVRSD_UINT32, &val);
  return val;
}

void
_Unwind_SetGR (_Unwind_Context *context, int regno, _Unwind_Word val)
{
  _Unwind_VRS_Set (context, _UVRSC_CORE, regno, _UVRSD_UINT32, &val);
}

/* leb128 type numbers have a potentially unlimited size.
   The target of the following definitions of _sleb128_t and _uleb128_t
   is to have efficient data types large enough to hold the leb128 type
   numbers used in the unwind code.  */
alias _sleb128_t = __builtin_clong;
alias _uleb128_t = __builtin_culong;

enum int UNWIND_STACK_REG = 13;
/* Use IP as a scratch register within the personality routine.  */
enum int UNWIND_POINTER_REG = 12;

version (linux)
  const ubyte _TTYPE_ENCODING = (DW_EH_PE_pcrel | DW_EH_PE_indirect);
else version (NetBSD)
  const ubyte _TTYPE_ENCODING = (DW_EH_PE_pcrel | DW_EH_PE_indirect);
else version (symbian) // TODO: name
  const ubyte _TTYPE_ENCODING = (DW_EH_PE_absptr);
else version (uclinux) // TODO: name
  const ubyte _TTYPE_ENCODING = (DW_EH_PE_absptr);
else
  const ubyte _TTYPE_ENCODING = (DW_EH_PE_pcrel);

/* Decode an R_ARM_TARGET2 relocation.  */
_Unwind_Word
_Unwind_decode_typeinfo_ptr (_Unwind_Word base, _Unwind_Word ptr)
{
  _Unwind_Word tmp;
  tmp = *cast(_Unwind_Word *) ptr;
  /* Zero values are always NULL.  */
  if (!tmp)
    return 0;

  if (_TTYPE_ENCODING == (DW_EH_PE_pcrel | DW_EH_PE_indirect))
    {
      /* Pc-relative indirect.  */
      tmp += ptr;
      tmp = *cast(_Unwind_Word *) tmp;
    }
  else if (_TTYPE_ENCODING == DW_EH_PE_absptr)
    {
      /* Absolute pointer.  Nothing more to do.  */
    }
  else
    {
      /* Pc-relative pointer.  */
      tmp += ptr;
    }
  return tmp;
}

_Unwind_Reason_Code
__gnu_unwind_24bit (_Unwind_Context * context, _uw data, int compact)
{
  return _URC_FAILURE;
}

/* Return the address of the instruction, not the actual IP value.  */
_Unwind_Word
_Unwind_GetIP(_Unwind_Context *context)
{
  return _Unwind_GetGR (context, 15) & ~ cast(_Unwind_Word) 1;
}

/* The dwarf unwinder doesn't understand arm/thumb state.  We assume the
   landing pad uses the same instruction set as the call site.  */
void
_Unwind_SetIP(_Unwind_Context *context, _Unwind_Word val)
{
  return _Unwind_SetGR (context, 15, val | (_Unwind_GetGR (context, 15) & 1));
}

