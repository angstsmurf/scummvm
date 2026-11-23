/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 53 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_fread
#define FORBIDDEN_SYMBOL_EXCEPTION_stdin
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_getc

#include "common/endian.h"
#include "common/hash-str.h"
#include "common/rect.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-the.h"

extern int yylex();

using namespace Director;

// if we have the trim garbage flag on, and we encounter
// an unwelcome surprise in an expression, use everything up until
// the first failing token.
#define TRIM_GARBAGE(src, target) \
	if (g_lingo->_compiler->_trimGarbage) { \
		warning("Trimming garbage and continuing.");	\
		src = target;	\
		yyerrok; \
	} else { \
		src = nullptr; \
		YYABORT; \
	}	\


static void yyerror(const char *s) {
	LingoCompiler *compiler = g_lingo->_compiler;
	compiler->_hadError = true;
	warning("%s  LINGO: %s at line %d col %d in %s id: %d",
		(g_director->_noFatalLingoError ? "####" : "######################"), s, compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
		compiler->_assemblyContext->_id);
	if (compiler->_lines[2] != compiler->_lines[1])
		warning("# %3d: %s", compiler->_linenumber - 2, Common::String(compiler->_lines[2], compiler->_lines[1] - 1).c_str());

	if (compiler->_lines[1] != compiler->_lines[0])
		warning("# %3d: %s", compiler->_linenumber - 1, Common::String(compiler->_lines[1], compiler->_lines[0] - 1).c_str());

	const char *ptr = compiler->_lines[0];

	while (*ptr && *ptr != '\n')
		ptr++;

	warning("# %3d: %s", compiler->_linenumber, Common::String(compiler->_lines[0], ptr).c_str());

	Common::String arrow;
	for (uint i = 0; i < compiler->_colnumber; i++)
		arrow += ' ';

	warning("#      %s^ about here", arrow.c_str());
}

static void checkEnd(Common::String *token, Common::String *expect, bool required) {
	if (required) {
		if (token->compareToIgnoreCase(*expect)) {
			Common::String err = Common::String::format("end mismatch. Expected %s but got %s", expect->c_str(), token->c_str());
			yyerror(err.c_str());
		}
	}
}


#line 150 "engines/director/lingo/lingo-gr.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "lingo-gr.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_tUNARY = 3,                     /* tUNARY  */
  YYSYMBOL_tINT = 4,                       /* tINT  */
  YYSYMBOL_tFLOAT = 5,                     /* tFLOAT  */
  YYSYMBOL_tVARID = 6,                     /* tVARID  */
  YYSYMBOL_tSTRING = 7,                    /* tSTRING  */
  YYSYMBOL_tSYMBOL = 8,                    /* tSYMBOL  */
  YYSYMBOL_tENDCLAUSE = 9,                 /* tENDCLAUSE  */
  YYSYMBOL_tCAST = 10,                     /* tCAST  */
  YYSYMBOL_tFIELD = 11,                    /* tFIELD  */
  YYSYMBOL_tSCRIPT = 12,                   /* tSCRIPT  */
  YYSYMBOL_tWINDOW = 13,                   /* tWINDOW  */
  YYSYMBOL_tMEMBER = 14,                   /* tMEMBER  */
  YYSYMBOL_tCASTLIB = 15,                  /* tCASTLIB  */
  YYSYMBOL_tDELETE = 16,                   /* tDELETE  */
  YYSYMBOL_tDOWN = 17,                     /* tDOWN  */
  YYSYMBOL_tELSE = 18,                     /* tELSE  */
  YYSYMBOL_tEXIT = 19,                     /* tEXIT  */
  YYSYMBOL_tFRAME = 20,                    /* tFRAME  */
  YYSYMBOL_tGLOBAL = 21,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 22,                       /* tGO  */
  YYSYMBOL_tHILITE = 23,                   /* tHILITE  */
  YYSYMBOL_tIF = 24,                       /* tIF  */
  YYSYMBOL_tIN = 25,                       /* tIN  */
  YYSYMBOL_tINTO = 26,                     /* tINTO  */
  YYSYMBOL_tMACRO = 27,                    /* tMACRO  */
  YYSYMBOL_tRETURN = 28,                   /* tRETURN  */
  YYSYMBOL_tMOVIE = 29,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 30,                     /* tNEXT  */
  YYSYMBOL_tOF = 31,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 32,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 33,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 34,                   /* tREPEAT  */
  YYSYMBOL_tSET = 35,                      /* tSET  */
  YYSYMBOL_tTHEN = 36,                     /* tTHEN  */
  YYSYMBOL_tTO = 37,                       /* tTO  */
  YYSYMBOL_tWHEN = 38,                     /* tWHEN  */
  YYSYMBOL_tWITH = 39,                     /* tWITH  */
  YYSYMBOL_tWHILE = 40,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 41,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 42,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 43,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 44,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 45,                       /* tGE  */
  YYSYMBOL_tLE = 46,                       /* tLE  */
  YYSYMBOL_tEQ = 47,                       /* tEQ  */
  YYSYMBOL_tNEQ = 48,                      /* tNEQ  */
  YYSYMBOL_tAND = 49,                      /* tAND  */
  YYSYMBOL_tOR = 50,                       /* tOR  */
  YYSYMBOL_tNOT = 51,                      /* tNOT  */
  YYSYMBOL_tMOD = 52,                      /* tMOD  */
  YYSYMBOL_tAFTER = 53,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 54,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 55,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 56,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 57,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 58,                     /* tCHAR  */
  YYSYMBOL_tCHARS = 59,                    /* tCHARS  */
  YYSYMBOL_tITEM = 60,                     /* tITEM  */
  YYSYMBOL_tITEMS = 61,                    /* tITEMS  */
  YYSYMBOL_tLINE = 62,                     /* tLINE  */
  YYSYMBOL_tLINES = 63,                    /* tLINES  */
  YYSYMBOL_tWORD = 64,                     /* tWORD  */
  YYSYMBOL_tWORDS = 65,                    /* tWORDS  */
  YYSYMBOL_tABBREVIATED = 66,              /* tABBREVIATED  */
  YYSYMBOL_tABBREV = 67,                   /* tABBREV  */
  YYSYMBOL_tABBR = 68,                     /* tABBR  */
  YYSYMBOL_tLONG = 69,                     /* tLONG  */
  YYSYMBOL_tSHORT = 70,                    /* tSHORT  */
  YYSYMBOL_tDATE = 71,                     /* tDATE  */
  YYSYMBOL_tLAST = 72,                     /* tLAST  */
  YYSYMBOL_tMENU = 73,                     /* tMENU  */
  YYSYMBOL_tMENUS = 74,                    /* tMENUS  */
  YYSYMBOL_tMENUITEM = 75,                 /* tMENUITEM  */
  YYSYMBOL_tMENUITEMS = 76,                /* tMENUITEMS  */
  YYSYMBOL_tNUMBER = 77,                   /* tNUMBER  */
  YYSYMBOL_tTHE = 78,                      /* tTHE  */
  YYSYMBOL_tTIME = 79,                     /* tTIME  */
  YYSYMBOL_tXTRAS = 80,                    /* tXTRAS  */
  YYSYMBOL_tCASTLIBS = 81,                 /* tCASTLIBS  */
  YYSYMBOL_tSOUND = 82,                    /* tSOUND  */
  YYSYMBOL_tSPRITE = 83,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 84,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 85,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 86,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 87,                 /* tPROPERTY  */
  YYSYMBOL_tON = 88,                       /* tON  */
  YYSYMBOL_tMETHOD = 89,                   /* tMETHOD  */
  YYSYMBOL_tENDIF = 90,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 91,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 92,                  /* tENDTELL  */
  YYSYMBOL_tASSERTERROR = 93,              /* tASSERTERROR  */
  YYSYMBOL_94_ = 94,                       /* '<'  */
  YYSYMBOL_95_ = 95,                       /* '>'  */
  YYSYMBOL_96_ = 96,                       /* '&'  */
  YYSYMBOL_97_ = 97,                       /* '+'  */
  YYSYMBOL_98_ = 98,                       /* '-'  */
  YYSYMBOL_99_ = 99,                       /* '*'  */
  YYSYMBOL_100_ = 100,                     /* '/'  */
  YYSYMBOL_101_n_ = 101,                   /* '\n'  */
  YYSYMBOL_102_ = 102,                     /* ','  */
  YYSYMBOL_103_ = 103,                     /* '('  */
  YYSYMBOL_104_ = 104,                     /* ')'  */
  YYSYMBOL_105_ = 105,                     /* '['  */
  YYSYMBOL_106_ = 106,                     /* ']'  */
  YYSYMBOL_107_ = 107,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 108,                 /* $accept  */
  YYSYMBOL_script = 109,                   /* script  */
  YYSYMBOL_scriptpartlist = 110,           /* scriptpartlist  */
  YYSYMBOL_scriptpart = 111,               /* scriptpart  */
  YYSYMBOL_macro = 112,                    /* macro  */
  YYSYMBOL_factory = 113,                  /* factory  */
  YYSYMBOL_method = 114,                   /* method  */
  YYSYMBOL_methodlist = 115,               /* methodlist  */
  YYSYMBOL_nonemptymethodlist = 116,       /* nonemptymethodlist  */
  YYSYMBOL_methodlistline = 117,           /* methodlistline  */
  YYSYMBOL_handler = 118,                  /* handler  */
  YYSYMBOL_endargdef = 119,                /* endargdef  */
  YYSYMBOL_CMDID = 120,                    /* CMDID  */
  YYSYMBOL_ID = 121,                       /* ID  */
  YYSYMBOL_idlist = 122,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 123,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 124,                     /* stmt  */
  YYSYMBOL_stmt_insideif = 125,            /* stmt_insideif  */
  YYSYMBOL_stmtoneliner = 126,             /* stmtoneliner  */
  YYSYMBOL_proc = 127,                     /* proc  */
  YYSYMBOL_cmdargs = 128,                  /* cmdargs  */
  YYSYMBOL_trailingcomma = 129,            /* trailingcomma  */
  YYSYMBOL_frameargs = 130,                /* frameargs  */
  YYSYMBOL_asgn = 131,                     /* asgn  */
  YYSYMBOL_to = 132,                       /* to  */
  YYSYMBOL_definevars = 133,               /* definevars  */
  YYSYMBOL_ifstmt = 134,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 135,               /* ifelsestmt  */
  YYSYMBOL_endif = 136,                    /* endif  */
  YYSYMBOL_loop = 137,                     /* loop  */
  YYSYMBOL_tell = 138,                     /* tell  */
  YYSYMBOL_when = 139,                     /* when  */
  YYSYMBOL_stmtlist = 140,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 141,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 142,             /* stmtlistline  */
  YYSYMBOL_stmtlist_insideif = 143,        /* stmtlist_insideif  */
  YYSYMBOL_nonemptystmtlist_insideif = 144, /* nonemptystmtlist_insideif  */
  YYSYMBOL_stmtlistline_insideif = 145,    /* stmtlistline_insideif  */
  YYSYMBOL_simpleexpr_nounarymath = 146,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 147,                      /* var  */
  YYSYMBOL_varorchunk = 148,               /* varorchunk  */
  YYSYMBOL_varorthe = 149,                 /* varorthe  */
  YYSYMBOL_chunk = 150,                    /* chunk  */
  YYSYMBOL_chunktype = 151,                /* chunktype  */
  YYSYMBOL_object = 152,                   /* object  */
  YYSYMBOL_refargs = 153,                  /* refargs  */
  YYSYMBOL_the = 154,                      /* the  */
  YYSYMBOL_theobj = 155,                   /* theobj  */
  YYSYMBOL_menu = 156,                     /* menu  */
  YYSYMBOL_thedatetime = 157,              /* thedatetime  */
  YYSYMBOL_thenumberof = 158,              /* thenumberof  */
  YYSYMBOL_inof = 159,                     /* inof  */
  YYSYMBOL_writablethe = 160,              /* writablethe  */
  YYSYMBOL_writabletheobj = 161,           /* writabletheobj  */
  YYSYMBOL_list = 162,                     /* list  */
  YYSYMBOL_proplist = 163,                 /* proplist  */
  YYSYMBOL_proppair = 164,                 /* proppair  */
  YYSYMBOL_unarymath = 165,                /* unarymath  */
  YYSYMBOL_simpleexpr = 166,               /* simpleexpr  */
  YYSYMBOL_expr = 167,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 168,         /* expr_nounarymath  */
  YYSYMBOL_expr_noeq = 169,                /* expr_noeq  */
  YYSYMBOL_sprite = 170,                   /* sprite  */
  YYSYMBOL_exprlist = 171,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 172          /* nonemptyexprlist  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  199
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5034

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  108
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  375
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  740

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   348


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     101,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    96,     2,
     103,   104,    99,    97,   102,    98,     2,   100,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   107,     2,
      94,     2,    95,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   105,     2,   106,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   225,   225,   227,   233,   240,   241,   242,   243,   244,
     245,   274,   275,   280,   281,   284,   285,   288,   289,   292,
     298,   305,   306,   307,   312,   316,   320,   322,   326,   327,
     328,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     408,   409,   410,   413,   417,   427,   428,   431,   432,   433,
     434,   435,   436,   439,   440,   441,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   480,   483,   488,   492,
     498,   503,   506,   511,   515,   521,   528,   528,   534,   539,
     545,   551,   557,   565,   566,   567,   568,   569,   570,   571,
     572,   575,   576,   577,   578,   581,   581,   583,   584,   585,
     586,   587,   588,   591,   595,   597,   601,   607,   611,   615,
     619,   626,   628,   630,   632,   634,   636,   638,   640,   642,
     646,   650,   652,   656,   658,   659,   663,   669,   676,   677,
     680,   681,   685,   691,   698,   699,   705,   706,   707,   708,
     709,   710,   711,   712,   718,   723,   724,   725,   726,   727,
     728,   731,   733,   734,   737,   738,   741,   742,   747,   748,
     753,   754,   755,   757,   759,   761,   763,   765,   767,   769,
     771,   774,   775,   776,   777,   780,   781,   784,   789,   792,
     797,   803,   804,   805,   806,   807,   810,   811,   812,   813,
     814,   817,   819,   820,   821,   822,   823,   824,   825,   826,
     827,   828,   832,   833,   834,   835,   836,   837,   838,   839,
     842,   842,   844,   845,   848,   849,   850,   851,   852,   855,
     856,   857,   863,   867,   870,   875,   876,   877,   878,   879,
     882,   883,   886,   887,   891,   892,   893,   894,   895,   896,
     897,   898,   899,   900,   901,   902,   903,   904,   905,   906,
     907,   908,   909,   916,   917,   918,   919,   920,   921,   922,
     923,   924,   925,   926,   927,   928,   929,   930,   931,   932,
     933,   934,   937,   938,   939,   940,   941,   942,   943,   944,
     945,   946,   947,   948,   949,   950,   951,   952,   953,   954,
     957,   958,   961,   962,   965,   969
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  static const char *const yy_sname[] =
  {
  "end of file", "error", "invalid token", "tUNARY", "tINT", "tFLOAT",
  "tVARID", "tSTRING", "tSYMBOL", "tENDCLAUSE", "tCAST", "tFIELD",
  "tSCRIPT", "tWINDOW", "tMEMBER", "tCASTLIB", "tDELETE", "tDOWN", "tELSE",
  "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tHILITE", "tIF", "tIN", "tINTO",
  "tMACRO", "tRETURN", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE",
  "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tCHARS", "tITEM", "tITEMS", "tLINE",
  "tLINES", "tWORD", "tWORDS", "tABBREVIATED", "tABBREV", "tABBR", "tLONG",
  "tSHORT", "tDATE", "tLAST", "tMENU", "tMENUS", "tMENUITEM", "tMENUITEMS",
  "tNUMBER", "tTHE", "tTIME", "tXTRAS", "tCASTLIBS", "tSOUND", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tMETHOD",
  "tENDIF", "tENDREPEAT", "tENDTELL", "tASSERTERROR", "'<'", "'>'", "'&'",
  "'+'", "'-'", "'*'", "'/'", "'\\n'", "','", "'('", "')'", "'['", "']'",
  "':'", "$accept", "script", "scriptpartlist", "scriptpart", "macro",
  "factory", "method", "methodlist", "nonemptymethodlist",
  "methodlistline", "handler", "endargdef", "CMDID", "ID", "idlist",
  "nonemptyidlist", "stmt", "stmt_insideif", "stmtoneliner", "proc",
  "cmdargs", "trailingcomma", "frameargs", "asgn", "to", "definevars",
  "ifstmt", "ifelsestmt", "endif", "loop", "tell", "when", "stmtlist",
  "nonemptystmtlist", "stmtlistline", "stmtlist_insideif",
  "nonemptystmtlist_insideif", "stmtlistline_insideif",
  "simpleexpr_nounarymath", "var", "varorchunk", "varorthe", "chunk",
  "chunktype", "object", "refargs", "the", "theobj", "menu", "thedatetime",
  "thenumberof", "inof", "writablethe", "writabletheobj", "list",
  "proplist", "proppair", "unarymath", "simpleexpr", "expr",
  "expr_nounarymath", "expr_noeq", "sprite", "exprlist",
  "nonemptyexprlist", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-597)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    3696,  -597,  4065,  -597,  -597,  -597,  -597,  -597,  -597,   167,
    -597,     9,  -597,  4065,  2782,   167,  2884,  -597,  -597,  4065,
    1352,  -597,    50,  -597,  -597,  2986,    11,  4150,  -597,   -25,
    -597,  -597,  4065,  2986,  2782,  4065,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  4065,  -597,  -597,
    -597,  -597,  -597,  2884,  4065,  4065,   -11,  4404,  -597,    98,
    3696,  -597,  -597,  -597,  -597,  2170,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,    57,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,   -49,  -597,  -597,  3088,  3088,  3088,  3088,
    2884,  2884,  2884,  2884,    40,    16,    14,    19,  -597,  -597,
      22,    30,  -597,  -597,  -597,  -597,  3088,  3088,  3088,  3088,
    3088,  3088,  2884,  2884,  3190,  2884,  2884,  2884,  2884,  4235,
    2884,  3190,  3190,  2272,  1454,    51,    23,    28,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  1558,  -597,
      34,  2884,   323,  4065,    35,  -597,   389,    36,    37,  1660,
    4065,  2884,  -597,  -597,    45,  -597,  -597,    38,    43,  1762,
      44,    46,    54,   118,   308,    56,  4065,  -597,  -597,  -597,
    -597,  2884,    58,  1864,  2884,  -597,  4065,  2374,  -597,   129,
    -597,  -597,  -597,   132,  -597,  1210,  4453,  4475,  4509,   134,
      69,  -597,  -597,    79,  -597,    84,  -597,  4065,  -597,  -597,
    1966,  4852,  -597,     6,    20,    29,    32,    42,   134,   156,
     162,  4839,  -597,  -597,  -597,  3598,  1223,    90,    92,    99,
     101,    80,   -10,     3,  -597,  4852,   103,   109,  2476,   112,
    -597,   114,  -597,   187,  2884,  2884,  2884,  2884,  2884,  2884,
    2884,  2884,  2884,  2884,  2884,  2884,  2884,  2884,  2884,  2884,
    2884,  2884,  -597,  -597,  4737,  -597,   123,  -597,  4677,  1015,
      60,  -597,   127,  -597,   138,  -597,   141,  -597,  4320,  4320,
    4320,    33,   540,  -597,  -597,  2884,   142,     5,   143,  -597,
    2884,   145,  -597,   157,  -597,   158,  -597,  3292,   159,  4404,
    3788,   163,  -597,    61,   607,   171,  -597,   669,  -597,  -597,
    4656,   207,   215,  3190,  2884,  3190,  2884,  3190,  2884,  3190,
    2884,  -597,  -597,  -597,  -597,    48,  -597,  -597,  -597,  -597,
     244,  4908,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  2068,  3394,  3190,  3190,  4750,  2578,  -597,  2884,
    2884,  2884,  2884,  -597,  2884,  3496,  -597,  -597,  2884,  -597,
    3598,   172,  -597,  -597,  2884,   121,   121,   121,   121,   412,
     412,  -597,    91,   121,   121,   121,   121,    91,    -4,    -4,
    -597,  -597,   172,  2884,  2884,  2884,  2884,  2884,  2884,  2884,
    2884,  2884,  2884,  2884,  2884,  2884,  2884,  2884,  2884,  2884,
    2884,  -597,  -597,   174,  1153,   258,   176,  3788,  -597,  -597,
    -597,    40,  -597,    65,  -597,    67,    68,  2884,  2884,   180,
    3788,   731,     5,  4065,  4065,  -597,  -597,  -597,     5,  -597,
    -597,  4821,  -597,  -597,  -597,  2884,  2884,  2884,  2884,  -597,
    -597,  3788,  -597,  -597,  -597,   190,  3788,  -597,  -597,   182,
    3788,   183,  -597,  -597,   184,  -597,  2680,  3190,  3190,  -597,
    4531,  -597,  4565,  -597,  4587,  -597,  4621,  -597,  -597,  3190,
    2884,    48,    48,    48,    48,  3190,  -597,  3190,    48,  -597,
    -597,  3190,  3190,  -597,  -597,  -597,  -597,  -597,  -597,  2884,
     188,  -597,   172,  4852,  4852,  4852,  4852,  4852,  -597,  4852,
    4852,  4763,  2884,   189,  4852,  -597,   121,   121,   121,   121,
     412,   412,  -597,    91,   121,   121,   121,   121,    91,    -4,
      -4,  -597,  -597,   172,  1153,  -597,  -597,    -2,  1153,  -597,
    3880,  3788,  -597,   185,  -597,   186,  -597,   193,  -597,   855,
     643,  3788,   204,   195,  -597,  -597,    17,  4065,  -597,  -597,
    -597,  4921,  -597,  4643,  4921,  4921,   208,   202,  -597,  3788,
     296,  -597,  -597,  -597,   172,  -597,  -597,  3190,  3190,  3190,
    3190,  -597,  4852,  3190,  3190,  3190,  3190,  -597,   275,   235,
    -597,  -597,   172,  -597,   209,  2884,   210,  -597,  -597,   225,
    3972,   216,  -597,  -597,  1153,  -597,  -597,  -597,  -597,  -597,
     217,  3788,   279,  2884,   230,   238,  -597,  -597,    70,  2884,
    2884,  2884,  2884,  2884,  2884,  2884,  2884,  2884,  2884,  2884,
    2884,  2884,  2884,  2884,  2884,   267,   245,  -597,   338,  4065,
     246,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,   276,
    3190,  -597,   247,  -597,   172,  -597,  -597,  1153,  -597,  -597,
     225,  3788,   257,  2884,   879,   260,  -597,   261,  3788,   155,
     155,   155,  4934,  4934,  -597,   104,   155,   155,   155,   155,
     104,    26,    26,  -597,  -597,  2884,  -597,  4065,    73,  -597,
    3190,  -597,   262,   225,  -597,   285,   266,   912,   280,  3788,
    -597,  3788,  -597,  4921,    82,  -597,  -597,  -597,  -597,   281,
    -597,   282,  3788,  3788,   286,  -597,  -597,  -597,  3788,   294,
     295,   287,   300,   291,   292,  -597,   293,  -597,  -597,  -597
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    31,    28,    37,    44,    66,    75,    56,    38,    42,
      43,     0,    45,   100,   146,    46,     0,    47,    49,     0,
       0,    60,    61,    63,    64,   146,    65,     0,    73,     0,
      76,    74,     0,   146,   146,   100,    35,    36,    39,    40,
      50,    51,    53,    54,    78,    79,    32,    33,    34,    55,
      68,    41,    52,    57,    58,    59,    62,    71,    72,    69,
      70,    48,    77,     0,   100,     0,     0,    67,     5,     0,
       2,     3,     6,     7,     8,   146,     9,   105,   107,   113,
     114,   115,   108,   109,   110,   111,   112,     0,    82,    42,
      81,    83,    85,    86,    46,    87,    89,    96,    61,    95,
      65,    97,    99,    84,    92,    93,    88,    71,    98,    94,
      91,    90,    67,     0,    80,    29,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   135,   103,
       0,   101,   216,   217,   219,   218,    37,    44,    66,    75,
      56,    38,    45,    60,     0,    39,    50,    53,    78,    71,
      70,     0,     0,     0,   372,   231,     0,     0,   312,   226,
     227,   228,   229,   264,   265,   230,   313,   314,   156,   315,
       0,     0,     0,   100,     0,   137,     0,     0,     0,   156,
       0,     0,   231,   234,     0,   235,   203,     0,     0,   156,
       0,     0,     0,   292,     0,     0,   100,   106,   145,     1,
       4,     0,     0,   156,     0,    10,     0,     0,   238,   257,
     236,   257,   240,   257,   241,     0,     0,     0,     0,     0,
       0,   141,   136,     0,   133,     0,   177,   102,   255,   256,
     158,   159,   220,    32,    33,    34,    55,    68,    52,    62,
     261,     0,   310,   311,   151,   226,     0,   216,   217,   219,
     218,     0,   231,     0,   302,   374,     0,   373,     0,     0,
     120,     0,   122,    63,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   157,   147,   333,   156,   334,     0,   143,     0,     0,
       0,   138,     0,   139,     0,   131,     0,   118,    49,    35,
      36,     0,     0,   175,   176,     0,     0,    17,     0,   128,
      76,     0,   124,     0,   126,     0,   181,     0,     0,     0,
     204,     0,   179,     0,     0,     0,   116,     0,    30,   258,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   251,   253,   254,   252,     0,   142,   134,   178,   104,
      63,   162,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,     0,     0,     0,     0,   156,     0,   225,     0,
       0,     0,     0,   300,     0,     0,   301,   299,     0,   221,
     226,   156,   121,   123,     0,   325,   326,   323,   324,   327,
     328,   320,   330,   331,   332,   322,   321,   329,   316,   317,
     318,   319,   156,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     157,   149,   144,     0,   210,   183,     0,   204,   140,   132,
     119,    71,   232,     0,   233,     0,     0,     0,     0,     0,
     204,     0,    17,    28,     0,    21,    22,    13,    18,    19,
     129,     0,   125,   127,   182,    57,    58,    69,    70,   293,
     294,   204,   200,   208,   209,     0,   205,   206,   180,     0,
     204,     0,   171,   117,     0,   173,     0,     0,     0,   242,
       0,   246,     0,   248,     0,   244,     0,   290,   291,     0,
       0,    40,    51,    54,    79,    57,   287,    58,    59,   288,
     289,    69,    70,   263,   267,   266,   262,   370,   371,   157,
       0,   152,   156,   308,   309,   307,   305,   306,   303,   304,
     375,   156,   157,     0,   161,   148,   344,   345,   342,   343,
     346,   347,   339,   349,   350,   351,   341,   340,   348,   335,
     336,   337,   338,   156,   210,   214,   215,   190,   211,   212,
       0,   204,    11,     0,   163,     0,   165,     0,   167,     0,
       0,   204,     0,     0,   169,    14,     0,   100,    20,   130,
     352,   295,   353,     0,   297,   298,     0,     0,   207,   204,
      26,   172,   174,   259,   156,   237,   239,     0,     0,     0,
       0,   250,   160,     0,     0,     0,     0,   271,     0,     0,
     269,   270,   156,   154,     0,   157,     0,   222,   150,   190,
       0,     0,   184,   213,   210,   186,    12,   164,   166,   168,
       0,   204,     0,     0,     0,     0,   170,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   201,    27,    28,
       0,   243,   247,   249,   245,   282,   284,   285,   283,     0,
       0,   286,     0,   153,   156,   223,   185,   210,   188,   191,
     190,   204,     0,     0,     0,     0,   192,     0,   204,   362,
     363,   361,   364,   365,   358,   367,   368,   369,   360,   359,
     366,   354,   355,   356,   357,     0,   202,    28,     0,   260,
       0,   155,     0,   190,   187,     0,     0,     0,     0,   204,
     193,   204,    15,   296,     0,    24,   268,   224,   189,     0,
     198,     0,   204,   204,     0,    16,    25,   199,   204,     0,
       0,     0,     0,     0,     0,   194,     0,   196,   195,   197
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -597,  -597,  -597,   317,  -597,  -597,  -597,   -47,  -597,   -52,
    -597,  -435,     0,    -1,   -32,  -597,     2,  -402,   -58,  -597,
       8,  -280,   363,  -597,  -597,  -597,  -597,  -597,  -596,  -597,
    -597,  -597,  -421,  -597,   -68,  -533,  -597,  -149,  -143,   -20,
    -172,  -597,    12,  -597,  -597,   130,  -597,    47,  -199,  -597,
    -597,  -258,   374,  -597,  -597,  -597,    39,  -597,   655,   480,
    -215,  -306,  -114,  -597,  -253
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    69,    70,    71,    72,    73,   446,   447,   448,   449,
      74,   113,   114,   155,   130,   131,   464,    77,    78,    79,
     156,   282,   157,    80,   305,    81,    82,    83,   612,    84,
      85,    86,   465,   466,   467,   547,   548,   549,   158,   159,
     433,   184,   160,   345,   161,   208,   162,   503,   504,   163,
     164,   489,    87,   459,   165,   253,   254,   166,   167,   255,
     284,   571,   169,   256,   257
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      75,   115,    76,   192,   421,   381,   552,   183,   566,   198,
     126,   609,   129,   666,   443,   351,   610,   220,   173,   562,
     223,   125,   546,   225,   259,   283,   182,   170,   402,   261,
     366,   187,   195,   178,   129,   286,   283,   294,   296,   306,
     576,   188,   190,   127,   308,   311,   283,   313,   270,   580,
     180,   181,   205,   206,   285,   315,   193,   321,   437,   325,
     283,   426,   469,   129,   196,   285,   553,    75,   555,   557,
      75,   677,    76,   487,   704,   285,   186,   352,   634,   488,
     438,   670,   303,   202,   177,   353,   510,   283,   611,   285,
     197,   354,   304,   258,   444,   279,   280,   374,   199,   355,
     356,   523,   283,   358,   204,   375,   445,   718,   357,   376,
     128,   359,   219,   360,   512,   222,   285,   221,   627,   206,
     224,   361,   525,   226,   260,   643,   644,   435,   436,   262,
     616,   285,   227,   245,   703,   287,   291,   295,   297,   307,
     624,   290,   546,   270,   309,   312,   546,   314,   240,   317,
     573,   574,   575,   252,   258,   316,   634,   322,   648,   326,
     331,   427,   470,   332,   323,   521,   554,   543,   556,   558,
     346,   678,   129,   270,   715,   206,   271,   116,   117,   301,
     347,   118,   119,   726,   206,   348,   373,   362,   277,   278,
     279,   280,   341,   363,   342,   129,   343,   369,   344,   370,
     672,   641,   642,   643,   644,   328,   371,   634,   372,   377,
     635,   378,   546,   382,   698,   383,   384,   276,   277,   278,
     279,   280,   477,   584,   422,   120,   349,   121,   428,   122,
     478,   123,   604,   593,   594,   595,   596,   283,   380,   429,
     599,   606,   430,   442,   450,   124,   452,   210,   212,   214,
     705,   640,   641,   642,   643,   644,   602,   712,   453,   454,
     461,   462,   714,   608,   468,   546,   285,   210,   228,   229,
     212,   214,   473,   490,   522,   544,   550,   551,   432,   432,
     432,   561,   577,   579,   581,   582,   617,   618,   724,    75,
     725,   425,   603,   607,   619,   625,   626,   182,   182,   182,
     646,   729,   730,   647,   650,   649,   659,   732,   660,   318,
     434,   434,   434,   663,   665,   611,   673,   669,   671,    75,
      75,   675,   662,   679,   680,   681,   682,   683,   684,   685,
     686,   687,   688,   689,   690,   691,   692,   693,   694,   676,
     695,   572,   572,   572,   572,   319,   696,   697,   706,   700,
     699,   701,   664,   264,   265,   266,   267,   268,   269,   289,
     270,   710,   711,   271,   272,   273,   717,   720,   264,   265,
     266,   267,   268,   269,   252,   270,   719,   731,   271,   272,
     273,   723,   727,   728,   702,   733,   734,   200,   735,   713,
     292,   736,   737,   738,   739,   565,   568,   191,   578,   613,
     661,   185,   274,   275,   276,   277,   278,   279,   280,   320,
     506,     0,     0,     0,   518,     0,     0,   274,   275,   276,
     277,   278,   279,   280,    75,     0,     0,    75,     0,     0,
       0,     0,     0,     0,   264,   265,   266,   267,   268,   269,
      75,   270,   115,   567,   271,   272,   273,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   264,   265,   266,
     267,    75,     0,     0,   270,     0,    75,   271,   272,   273,
      75,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   274,   275,   276,   277,   278,   279,   280,
     293,     0,     0,     0,   168,     0,   172,     0,     0,     0,
     176,     0,     0,     0,     0,   179,   274,   275,   276,   277,
     278,   279,   280,   189,   168,   572,   572,   572,   572,   572,
     572,   572,   572,   572,   572,   572,   572,   572,   572,   572,
     572,     0,     0,     0,     0,   628,     0,     0,     0,     0,
       0,   439,     0,   194,    75,     0,     0,     0,    75,     0,
      75,    75,   615,     0,     0,   203,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   129,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
       0,   572,     0,     0,     0,   264,   265,   266,   267,   268,
     269,     0,   270,     0,     0,   271,   272,   273,     0,     0,
     215,   216,   217,   218,     0,     0,     0,     0,   471,     0,
      75,     0,   668,     0,    75,     0,     0,     0,     0,     0,
       0,    75,   230,   231,     0,   215,   216,   217,   218,     0,
     241,     0,     0,   246,   274,   275,   276,   277,   278,   279,
     280,   440,     0,     0,     0,     0,     0,     0,   115,     0,
       0,   288,   264,   265,   266,   267,   268,   269,     0,   270,
     622,   302,   271,   272,   273,     0,     0,    75,     0,     0,
     474,    75,     0,     0,     0,     0,     0,     0,    75,     0,
     623,   324,     0,     0,   327,     0,     0,   330,   264,   265,
     266,   267,   268,   269,     0,   270,   115,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   472,    75,
       0,    75,     0,     0,   264,   265,   266,   267,   268,   269,
       0,   270,    75,    75,   271,   272,   273,     0,    75,     0,
       0,     0,   563,     0,     0,     0,     0,   274,   275,   276,
     277,   278,   279,   280,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,     0,     0,   274,   275,   276,   277,   278,   279,   280,
     475,   209,   211,   213,   211,     0,   264,   265,   266,   267,
     268,   269,     0,   270,     0,   441,   271,   272,   273,     0,
     451,   209,   211,   211,   211,   213,   211,     0,     0,   232,
       0,     0,     0,     0,     0,     0,   242,   243,     0,     0,
       0,     0,     0,     0,   480,     0,   482,     0,   484,     0,
     486,     0,     0,     0,     0,   274,   275,   276,   277,   278,
     279,   280,   564,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   513,
     514,   515,   516,     0,   517,   519,   620,     0,   520,     0,
       0,     0,     0,     0,   524,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     708,     0,     0,   526,   527,   528,   529,   530,   531,   532,
     533,   534,   535,   536,   537,   538,   539,   540,   541,   542,
     264,   265,   266,   267,   268,   269,     0,   270,     0,     0,
     271,   272,   273,   721,     0,     0,     0,   559,   560,     0,
       0,     0,     0,     0,   264,   265,   266,   267,   268,   269,
       0,   270,     0,     0,   271,   272,   273,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   274,
     275,   276,   277,   278,   279,   280,   621,   264,   265,   266,
     267,   268,   269,     0,   270,     0,     0,   271,   272,   273,
     592,     0,   460,   274,   275,   276,   277,   278,   279,   280,
     709,     0,     0,     0,     0,     0,     0,     0,   479,     0,
     481,     0,   483,     0,   485,     0,     0,     0,     0,     0,
       0,     0,   520,     0,     0,     0,   274,   275,   276,   277,
     278,   279,   280,   722,     0,     0,   423,   505,   505,   507,
     508,     1,     0,     0,     0,     3,     4,     5,     6,     7,
       8,     9,    10,     0,    11,    12,    13,    14,    15,    16,
      17,    18,     0,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,    33,    34,    35,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,    57,    58,     0,     0,    59,    60,    61,
      62,    63,    64,   674,     0,    66,     0,     0,    67,     0,
     570,   570,   570,   570,     0,     0,   424,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   585,   586,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   591,     0,     0,     0,     0,     0,
     597,     0,   598,   707,     0,     0,   600,   601,     0,     1,
       0,     0,     0,     3,     4,     5,     6,     7,     8,     9,
      10,     0,    11,    12,    13,    14,    15,    16,    17,    18,
       0,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,    33,    34,    35,     0,     0,
       0,     0,     0,     0,     0,     0,    36,    37,     0,     0,
       0,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,    57,    58,     0,     0,    59,    60,    61,    62,    63,
      64,   333,   651,   652,   653,   654,    67,   334,   655,   656,
     657,   658,     0,     0,   545,   264,   265,   266,   267,   268,
     269,     0,   270,     0,     0,   271,   272,   273,   264,   265,
     266,   267,   268,   269,     0,   270,     0,     0,   271,   272,
     273,     0,     0,     0,   570,   570,   570,   570,   570,   570,
     570,   570,   570,   570,   570,   570,   570,   570,   570,   570,
       0,     0,     0,     0,   274,   275,   276,   277,   278,   279,
     280,     0,     0,     0,     0,   597,     0,   274,   275,   276,
     277,   278,   279,   280,     0,   367,     0,   368,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     570,     0,     0,   174,     0,   716,   132,   133,     1,   134,
     135,    88,   136,   137,   138,   139,   140,   141,    89,    10,
      90,    91,    12,    92,    93,    94,    95,    17,    18,    96,
      97,    21,    98,    23,    24,    99,   100,   101,   102,    28,
       0,    30,    31,   103,   104,   105,   106,     0,     0,     0,
       0,     0,     0,   144,     0,    36,    37,     0,     0,     0,
     145,    39,   146,    41,   147,    43,   148,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,    54,    55,    56,
     149,    58,     0,     0,    59,   150,    61,    62,   108,   109,
     110,   111,     0,     0,     0,   112,     0,     0,     0,   151,
     152,     0,     0,   175,     0,   171,     0,   154,   247,   248,
       1,   249,   250,    88,   136,   137,   138,   139,   140,   141,
      89,    10,    90,    91,    12,    92,    93,    94,    95,    17,
      18,    96,    97,    21,    98,    23,    24,    99,   100,   101,
     102,    28,     0,    30,    31,   103,   104,   105,   106,     0,
       0,     0,     0,     0,     0,   144,     0,    36,    37,     0,
       0,     0,   145,    39,   146,    41,   147,    43,   148,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,    54,
      55,    56,   149,    58,     0,     0,    59,   150,    61,    62,
     108,   109,   110,   111,     0,     0,     0,   112,     0,     0,
       0,   151,   152,     0,     0,     0,     0,   171,     0,   154,
       0,   251,   132,   133,     1,   134,   135,    88,   136,   137,
     138,   139,   140,   141,    89,    10,    90,    91,    12,    92,
      93,    94,    95,    17,    18,    96,    97,    21,    98,   263,
      24,    99,   100,   101,   102,    28,     0,    30,    31,   103,
     104,   105,   106,   264,   265,   266,   267,   268,   269,   144,
     270,    36,    37,   271,   272,   273,   145,    39,   146,    41,
     147,    43,   148,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,    54,    55,    56,   149,    58,     0,     0,
      59,   150,    61,    62,   108,   109,   110,   111,     0,     0,
       0,   112,   274,   275,   276,   277,   278,   279,   280,     0,
     281,   171,     0,   154,   132,   133,     1,   134,   135,    88,
     136,   137,   138,   139,   140,   141,    89,    10,    90,    91,
      12,    92,    93,    94,    95,    17,   298,    96,    97,    21,
      98,    23,    24,    99,   100,   101,   102,    28,     0,    30,
      31,   103,   104,   105,   106,   264,   265,   266,   267,   268,
     269,   144,   270,   299,   300,   271,   272,   273,   145,    39,
     146,    41,   147,    43,   148,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,   149,    58,
       0,     0,    59,   150,    61,    62,   108,   109,   110,   111,
       0,     0,     0,   112,   274,   275,   276,   277,   278,   279,
     280,     0,   281,   171,     0,   154,   132,   133,     1,   134,
     135,    88,   136,   137,   138,   139,   140,   141,    89,    10,
      90,    91,    12,    92,    93,    94,    95,    17,    18,    96,
      97,    21,    98,    23,    24,    99,   100,   101,   102,    28,
       0,   310,    31,   103,   104,   105,   106,   264,   265,   266,
     267,   268,   269,   144,   270,    36,    37,   271,   272,   273,
     145,    39,   146,    41,   147,    43,   148,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,    54,    55,    56,
     149,    58,     0,     0,    59,   150,    61,    62,   108,   109,
     110,   111,     0,     0,     0,   112,   274,   275,   276,   277,
     278,   279,   280,     0,   281,   171,     0,   154,   132,   133,
       1,   134,   135,    88,   136,   137,   138,   139,   140,   141,
      89,    10,    90,    91,    12,    92,    93,    94,    95,    17,
      18,    96,    97,    21,    98,    23,    24,    99,   100,   101,
     102,    28,     0,    30,    31,   103,   104,   105,   106,   264,
     265,   266,   267,   268,   269,   144,   270,    36,    37,   271,
     272,   273,   145,    39,   146,    41,   147,    43,   148,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,    54,
      55,    56,   149,    58,     0,     0,    59,   150,    61,    62,
     108,   109,   110,   111,     0,     0,     0,   112,   274,   275,
     276,   277,   278,   279,   280,     0,   281,   171,     0,   154,
     132,   133,     1,   134,   135,    88,   136,   137,   138,   139,
     140,   141,    89,    10,    90,    91,    12,    92,    93,    94,
      95,    17,    18,    96,    97,    21,    98,   350,    24,    99,
     100,   101,   102,    28,     0,    30,    31,   103,   104,   105,
     106,   264,   265,   266,   267,   268,   269,   144,   270,    36,
      37,   271,   272,   273,   145,    39,   146,    41,   147,    43,
     148,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,    54,    55,    56,   149,    58,     0,     0,    59,   150,
      61,    62,   108,   109,   110,   111,     0,     0,     0,   112,
     274,   275,   276,   277,   278,   279,   280,     0,     0,   171,
       0,   154,   132,   133,     1,   134,   135,    88,   136,   137,
     138,   139,   140,   141,    89,    10,    90,    91,    12,    92,
      93,    94,    95,    17,    18,    96,    97,    21,    98,    23,
      24,    99,   100,   101,   102,    28,     0,    30,    31,   103,
     104,   105,   106,     0,     0,     0,     0,     0,     0,   144,
       0,    36,    37,     0,     0,     0,   145,   491,   146,   492,
     147,   493,   148,   494,    46,    47,    48,    49,    50,    51,
      52,   495,   496,   497,   498,    56,   149,    58,   499,   500,
     501,   502,    61,    62,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,   151,   152,     0,     0,     0,
       0,   171,     0,   154,   132,   133,     1,   134,   135,    88,
     136,   137,   138,   139,   140,   141,    89,    10,    90,    91,
      12,    92,    93,    94,    95,    17,    18,    96,    97,    21,
      98,    23,    24,    99,   100,   101,   102,    28,     0,    30,
      31,   103,   104,   105,   106,     0,     0,   201,     0,     0,
       0,   144,     0,    36,    37,     0,     0,     0,   145,    39,
     146,    41,   147,    43,   148,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,   149,    58,
       0,     0,    59,   150,    61,    62,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   151,   152,     0,
       0,     0,     0,   153,     0,   154,   132,   133,     1,   134,
     135,    88,   136,   137,   138,   139,   140,   141,    89,    10,
      90,    91,    12,    92,    93,    94,    95,    17,    18,    96,
      97,    21,    98,    23,    24,    99,   100,   101,   102,    28,
       0,    30,    31,   103,   104,   105,   106,     0,     0,     0,
       0,     0,     0,   144,     0,    36,    37,     0,     0,     0,
     145,    39,   146,    41,   147,    43,   148,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,    54,    55,    56,
     149,    58,     0,     0,    59,   150,    61,    62,   108,   109,
     110,   111,     0,     0,     0,   112,     0,     0,     0,   151,
     152,     0,     0,     0,     0,   171,   244,   154,   132,   133,
       1,   134,   135,    88,   136,   137,   138,   139,   140,   141,
      89,    10,    90,    91,    12,    92,    93,    94,    95,    17,
      18,    96,    97,    21,    98,    23,    24,    99,   100,   101,
     102,    28,     0,    30,    31,   103,   104,   105,   106,     0,
       0,     0,     0,     0,     0,   144,     0,    36,    37,     0,
       0,     0,   145,    39,   146,    41,   147,    43,   148,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,    54,
      55,    56,   149,    58,     0,     0,    59,   150,    61,    62,
     108,   109,   110,   111,     0,     0,     0,   112,     0,     0,
       0,   151,   152,     0,     0,     0,     0,   171,   329,   154,
     132,   133,     1,   134,   135,    88,   136,   137,   138,   139,
     140,   141,    89,    10,    90,    91,    12,    92,    93,    94,
      95,    17,    18,    96,    97,    21,    98,    23,    24,    99,
     100,   101,   102,    28,     0,    30,    31,   103,   104,   105,
     106,     0,     0,     0,     0,     0,     0,   144,     0,    36,
      37,     0,     0,     0,   145,    39,   146,    41,   147,    43,
     148,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,    54,    55,    56,   149,    58,     0,     0,    59,   150,
      61,    62,   108,   109,   110,   111,     0,     0,     0,   112,
       0,     0,     0,   151,   152,     0,     0,     0,     0,   171,
     379,   154,   132,   133,     1,   134,   135,    88,   136,   137,
     138,   139,   140,   141,    89,    10,    90,    91,    12,    92,
      93,    94,    95,    17,    18,    96,    97,    21,    98,    23,
      24,    99,   100,   101,   102,    28,     0,    30,    31,   103,
     104,   105,   106,     0,     0,     0,     0,     0,     0,   144,
       0,    36,    37,     0,     0,     0,   145,    39,   146,    41,
     147,    43,   148,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,    54,    55,    56,   149,    58,     0,     0,
      59,   150,    61,    62,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,   151,   152,     0,     0,     0,
       0,   171,   511,   154,   132,   133,     1,   134,   135,    88,
     136,   137,   138,   139,   140,   141,    89,    10,    90,    91,
      12,    92,    93,    94,    95,    17,    18,    96,    97,    21,
      98,    23,    24,    99,   100,   101,   102,    28,     0,    30,
      31,   103,   104,   105,   106,     0,     0,     0,     0,     0,
       0,   144,     0,    36,    37,     0,     0,     0,   145,    39,
     146,    41,   147,    43,   148,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,   149,    58,
       0,     0,    59,   150,    61,    62,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   151,   152,     0,
       0,     0,     0,   171,   583,   154,   132,   133,     1,   134,
     135,    88,   136,   137,   138,   139,   140,   141,    89,    10,
      90,    91,   142,    92,    93,    94,    95,    17,    18,    96,
      97,   143,    98,    23,    24,    99,   100,   101,   102,    28,
       0,    30,    31,   103,   104,   105,   106,     0,     0,     0,
       0,     0,     0,   144,     0,    36,    37,     0,     0,     0,
     145,    39,   146,    41,   147,    43,   148,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,    54,    55,    56,
     149,    58,     0,     0,    59,   150,    61,    62,   108,   109,
     110,   111,     0,     0,     0,   112,     0,     0,     0,   151,
     152,     0,     0,     0,     0,   153,     0,   154,   132,   133,
       1,   134,   135,    88,   136,   137,   138,   139,   140,   141,
      89,    10,    90,    91,    12,    92,    93,    94,    95,    17,
      18,    96,    97,    21,    98,    23,    24,    99,   100,   101,
     102,    28,     0,    30,    31,   103,   104,   105,   106,     0,
       0,     0,     0,     0,     0,   144,     0,    36,    37,     0,
       0,     0,   145,    39,   146,    41,   147,    43,   148,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,    54,
      55,    56,   149,    58,     0,     0,    59,   150,    61,    62,
     108,   109,   110,   111,     0,     0,     0,   112,     0,     0,
       0,   151,   152,     0,     0,     0,     0,   171,     0,   154,
     132,   133,     1,   134,   135,    88,   136,   137,   138,   139,
     140,   141,    89,    10,    90,    91,    12,    92,    93,    94,
      95,    17,    18,    96,    97,    21,    98,    23,    24,    99,
     100,   101,   102,    28,     0,    30,    31,   103,   104,   105,
     106,     0,     0,     0,     0,     0,     0,   144,     0,    36,
      37,     0,     0,     0,   145,    39,   146,    41,   147,    43,
     148,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,    54,    55,    56,   149,    58,     0,     0,    59,   150,
      61,    62,   108,   109,   110,   111,     0,     0,     0,   112,
       0,     0,     0,   151,   152,     0,     0,     0,     0,   153,
       0,   154,   132,   133,     1,   134,   135,    88,   136,   137,
     138,   139,   140,   141,    89,    10,    90,    91,    12,    92,
      93,    94,    95,    17,    18,    96,    97,    21,    98,    23,
      24,    99,   100,   101,   102,    28,     0,    30,    31,   103,
     104,   105,   106,     0,     0,     0,     0,     0,     0,   144,
       0,    36,    37,     0,     0,     0,   145,    39,   146,    41,
     147,    43,   148,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,    54,    55,    56,   149,    58,     0,     0,
      59,    60,    61,    62,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,   151,   152,     0,     0,     0,
       0,   207,     0,   154,   132,   133,     1,   134,   135,    88,
     136,   137,   138,   139,   140,   141,    89,    10,    90,    91,
      12,    92,    93,    94,    95,    17,    18,    96,    97,    21,
      98,    23,    24,    99,   100,   101,   102,    28,     0,    30,
      31,   103,   104,   105,   106,     0,     0,     0,     0,     0,
       0,   144,     0,    36,    37,     0,     0,     0,   145,    39,
     146,    41,   147,    43,   148,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,   149,    58,
       0,     0,    59,    60,    61,    62,   108,   109,   110,   111,
       0,     0,     0,   112,     0,     0,     0,   151,   152,     0,
       0,     0,     0,   171,     0,   154,   132,   133,     1,   134,
     135,    88,   136,   137,   138,   139,   140,   141,    89,    10,
      90,    91,    12,    92,    93,    94,    95,    17,    18,    96,
      97,    21,    98,    23,    24,    99,   100,   101,   102,    28,
       0,    30,    31,   103,   104,   105,   106,     0,     0,     0,
       0,     0,     0,   144,     0,    36,    37,     0,     0,     0,
     145,    39,   146,    41,   147,    43,   148,    45,    46,    47,
      48,    49,    50,    51,    52,   455,     0,   456,    55,    56,
     149,    58,     0,     0,   457,   458,    61,    62,   108,   109,
     110,   111,     0,     0,     0,   112,     0,     0,     0,   151,
     152,     0,     0,     0,     0,   171,     0,   154,   132,   133,
       1,   134,   135,    88,   136,   137,   138,   139,   140,   141,
      89,    10,    90,    91,    12,    92,    93,    94,    95,    17,
      18,    96,    97,    21,    98,    23,    24,    99,   100,   101,
     102,    28,     0,    30,    31,   103,   104,   105,   106,     0,
       0,     0,     0,     0,     0,   144,     0,    36,    37,     0,
       0,     0,   145,    39,   146,    41,   147,    43,   148,    45,
      46,    47,    48,    49,    50,    51,    52,   495,     0,   497,
      55,    56,   149,    58,     0,     0,   501,   502,    61,    62,
     108,   109,   110,   111,     0,     0,     0,   112,     0,     0,
       0,   151,   152,     0,     0,     0,     0,   171,     0,   154,
     247,   248,     1,   249,   250,    88,   136,   137,   138,   139,
     140,   141,    89,    10,    90,    91,    12,    92,    93,    94,
      95,    17,    18,    96,    97,    21,    98,    23,    24,    99,
     100,   101,   102,    28,     0,    30,    31,   103,   104,   105,
     106,     0,     0,     0,     0,     0,     0,   144,     0,    36,
      37,     0,     0,     0,   145,    39,   146,    41,   147,    43,
     148,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,    54,    55,    56,   149,    58,     0,     0,    59,   150,
      61,    62,   108,   109,   110,   111,     0,     0,     0,   112,
       0,     0,     0,   151,   152,     0,     0,     0,     0,   171,
       0,   154,   132,   133,     1,   134,   135,    88,   136,   137,
     138,   139,   140,   141,    89,    10,    90,    91,    12,    92,
      93,    94,    95,    17,    18,    96,    97,    21,    98,    23,
      24,    99,   100,   101,   102,    28,     0,    30,    31,   103,
     104,   105,   106,     0,     0,     0,     0,     0,     0,   144,
       0,    36,    37,     0,     0,     0,   145,    39,   146,    41,
     147,    43,   148,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,    54,    55,    56,   149,    58,     0,     0,
      59,   150,    61,    62,   108,   109,   110,   111,     0,     0,
       0,   112,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   171,     1,   154,     0,     2,     3,     4,     5,     6,
       7,     8,     9,    10,     0,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,     0,     0,     0,    36,
      37,     0,     0,     0,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,    54,    55,    56,    57,    58,     0,     0,    59,    60,
      61,    62,    63,    64,    65,     0,    66,     0,     0,    67,
       0,     0,     0,     0,     1,     0,     0,    68,     3,     4,
       5,     6,     7,     8,     9,    10,     0,    11,    12,    13,
      14,    15,    16,    17,    18,     0,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
      33,    34,    35,     0,     0,     0,     0,     0,     0,     0,
       0,    36,    37,     0,     0,     0,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,    54,    55,    56,    57,    58,     0,     0,
      59,    60,    61,    62,    63,    64,     0,     0,    66,     0,
       0,    67,     0,     0,     0,     0,     1,     0,     0,   463,
       3,     4,     5,     6,     7,     8,     9,    10,     0,    11,
      12,    13,    14,    15,    16,    17,    18,     0,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,    33,    34,    35,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,     0,     0,     0,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,    57,    58,
       0,     0,    59,    60,    61,    62,    63,    64,     0,     0,
      66,     0,     0,    67,     0,     0,     0,     0,     1,     0,
       0,   614,     3,     4,     5,     6,     7,     8,     9,    10,
       0,    11,    12,    13,    14,    15,    16,    17,    18,     0,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,    33,    34,    35,     0,     0,     0,
       0,     0,     0,     0,     0,    36,    37,     0,     0,     0,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,    54,    55,    56,
      57,    58,     0,     0,    59,    60,    61,    62,    63,    64,
       0,     0,    66,     0,     0,    67,     0,     0,     0,     0,
       0,     1,     0,   667,    88,     3,     4,     5,     6,     7,
       8,    89,    10,    90,    91,    12,    92,    93,    94,    95,
      17,    18,    96,    97,    21,    98,    23,    24,    99,   100,
     101,   102,    28,     0,    30,    31,   103,   104,   105,   106,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   107,    58,     0,     0,    59,    60,    61,
      62,   108,   109,   110,   111,     0,     1,     0,   112,    88,
       3,     4,     5,     6,     7,     8,    89,    10,    90,    91,
      12,    92,    93,    94,    95,    17,    18,    96,    97,    21,
      98,    23,    24,    99,   100,   101,   102,    28,     0,    30,
      31,   103,   104,   105,   106,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,     0,     0,     0,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,    57,    58,
       0,     0,    59,    60,    61,    62,   108,   109,   110,   111,
       0,     1,     0,   112,    88,     3,     4,     5,     6,     7,
       8,    89,    10,    90,    91,    12,    92,    93,    94,    95,
      17,    18,    96,    97,    21,    98,    23,    24,    99,   100,
     101,   102,    28,     0,    30,    31,   103,   104,   105,   106,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,    38,    39,    40,    41,    42,    43,    44,
      45,   233,   234,   235,   236,   237,    51,   238,    53,     0,
      54,    55,   239,   107,    58,     0,     0,    59,    60,    61,
      62,   108,   109,   110,   111,     0,     1,     0,   112,    88,
     136,   137,     5,     6,   140,   141,    89,    10,    90,    91,
      12,    92,    93,    94,    95,    17,    18,    96,    97,    21,
      98,    23,    24,    99,   100,   101,   102,    28,     0,    30,
      31,   103,   104,   105,   106,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,     0,     0,     0,   145,    39,
     146,    41,   147,    43,   148,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,   431,    58,
       0,     0,    59,    60,    61,    62,   108,   109,   110,   111,
       1,     0,     0,   112,     3,     4,     5,     6,     7,     8,
       9,    10,     0,    11,    12,    13,    14,    15,     0,    17,
      18,     0,    20,    21,    22,    23,    24,    25,   100,    27,
       0,    28,     0,    30,    31,     0,    33,    34,    35,     0,
       0,     0,     0,     0,     0,     0,     0,    36,    37,     0,
       0,     0,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,    54,
      55,    56,    57,    58,   335,     0,    59,    60,    61,    62,
     336,    64,     0,     0,     0,     0,     0,    67,   264,   265,
     266,   267,   268,   269,     0,   270,   337,     0,   271,   272,
     273,     0,   338,     0,     0,     0,     0,     0,     0,     0,
     264,   265,   266,   267,   268,   269,     0,   270,     0,     0,
     271,   272,   273,     0,     0,     0,     0,     0,     0,     0,
     339,     0,     0,     0,     0,     0,   340,   274,   275,   276,
     277,   278,   279,   280,   264,   265,   266,   267,   268,   269,
       0,   270,   587,     0,   271,   272,   273,     0,     0,   274,
     275,   276,   277,   278,   279,   280,   264,   265,   266,   267,
     268,   269,     0,   270,     0,     0,   271,   272,   273,     0,
       0,     0,     0,     0,     0,     0,   588,     0,     0,     0,
       0,     0,     0,   274,   275,   276,   277,   278,   279,   280,
     264,   265,   266,   267,   268,   269,     0,   270,   589,     0,
     271,   272,   273,     0,     0,   274,   275,   276,   277,   278,
     279,   280,   264,   265,   266,   267,   268,   269,     0,   270,
       0,     0,   271,   272,   273,     0,     0,     0,     0,     0,
       0,     0,   590,     0,     0,     0,     0,     0,     0,   274,
     275,   276,   277,   278,   279,   280,   264,   265,   266,   267,
     268,   269,     0,   270,   645,     0,   271,   272,   273,     0,
       0,   274,   275,   276,   277,   278,   279,   280,   629,   630,
       0,   631,   632,   633,     0,   634,     0,     0,   635,   636,
     637,   264,   265,   266,   267,   268,   269,     0,   270,     0,
       0,   271,   272,   273,     0,   274,   275,   276,   277,   278,
     279,   280,   264,   265,   266,   267,   268,   269,     0,   270,
       0,     0,   271,   272,   273,     0,     0,   638,   639,   640,
     641,   642,   643,   644,     0,     0,     0,     0,     0,     0,
     274,   275,   276,   277,   278,   279,   280,     0,   476,     0,
     368,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   274,   275,   276,   277,   278,   279,   280,     0,     0,
       0,   368,   403,   404,   405,   406,   407,   408,     0,   409,
       0,     0,   410,   411,   412,   403,   404,   405,   406,   407,
     408,     0,   409,     0,     0,   410,   411,   412,   403,   404,
     405,   406,   407,   408,     0,   409,     0,     0,   410,   411,
     412,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   413,   414,   415,   416,   417,   418,   419,     0,   420,
       0,     0,     0,     0,   413,   414,   415,   416,   417,   418,
     419,     0,   509,     0,     0,     0,     0,   413,   414,   415,
     416,   417,   418,   419,     0,   605,   264,   265,   266,   267,
     268,   269,     0,   270,     0,     0,   271,   272,   273,     0,
       0,     0,     0,     0,   264,   265,   266,   267,   268,   269,
       0,   270,     0,     0,   271,   272,   273,   264,   265,   266,
     267,   268,   269,     0,   270,     0,     0,   271,   272,   273,
       0,     0,     0,     0,     0,   274,   275,   276,   277,   278,
     279,   280,   569,   364,   365,     0,     0,     0,     0,     0,
       0,     0,     0,   274,   275,   276,   277,   278,   279,   280,
       0,     0,     0,     0,     0,     0,   274,   275,   276,   277,
     278,   279,   280,   403,   404,   405,   406,   407,   408,     0,
     409,     0,     0,   410,   411,   412,   629,   630,     0,   631,
     632,   633,     0,   634,     0,     0,   635,   636,   637,   629,
     630,     0,   631,     0,     0,     0,   634,     0,     0,   635,
     636,   637,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   413,   414,   415,   416,   417,   418,   419,     0,
       0,     0,     0,     0,     0,   638,   639,   640,   641,   642,
     643,   644,     0,     0,     0,     0,     0,     0,   638,   639,
     640,   641,   642,   643,   644
};

static const yytype_int16 yycheck[] =
{
       0,     2,     0,    35,   284,   258,   427,    27,   443,    67,
       1,   544,    13,   609,     9,   230,    18,     1,    19,   440,
       1,     9,   424,     1,     1,   168,    27,    15,   281,     1,
     245,    32,    64,    25,    35,     1,   179,     1,     1,     1,
     461,    33,    34,    34,     1,     1,   189,     1,    52,   470,
      39,    40,   101,   102,   168,     1,    57,     1,    25,     1,
     203,     1,     1,    64,    65,   179,     1,    67,     1,     1,
      70,     1,    70,    25,   670,   189,   101,    71,    52,    31,
      47,   614,    37,    75,    34,    79,   366,   230,    90,   203,
     101,    71,    47,   103,    89,    99,   100,   107,     0,    79,
      71,   381,   245,    71,    47,   102,   101,   703,    79,   106,
     101,    79,    72,    71,   367,   101,   230,   101,   101,   102,
     101,    79,   402,   101,   101,    99,   100,   299,   300,   101,
     551,   245,   102,   153,   667,   101,   101,   101,   101,   101,
     561,   173,   544,    52,   101,   101,   548,   101,   149,    31,
     456,   457,   458,   154,   103,   101,    52,   101,   579,   101,
      31,   101,   101,    31,   196,   380,   101,   420,   101,   101,
     101,   101,   173,    52,   101,   102,    55,    10,    11,   180,
     101,    14,    15,   101,   102,   101,   106,    31,    97,    98,
      99,   100,    58,    31,    60,   196,    62,   107,    64,   107,
     621,    97,    98,    99,   100,   206,   107,    52,   107,   106,
      55,   102,   614,   101,   649,   101,    29,    96,    97,    98,
      99,   100,    15,   476,   101,    58,   227,    60,   101,    62,
      15,    64,   512,   491,   492,   493,   494,   380,   258,   101,
     498,   521,   101,   101,   101,    78,   101,   117,   118,   119,
     671,    96,    97,    98,    99,   100,   509,   678,   101,   101,
     101,   319,   697,   543,   101,   667,   380,   137,   138,   139,
     140,   141,   101,    29,   102,   101,    18,   101,   298,   299,
     300,   101,    92,   101,   101,   101,   101,   101,   709,   289,
     711,   289,   104,   104,   101,    91,   101,   298,   299,   300,
      92,   722,   723,   101,   584,     9,    31,   728,    73,     1,
     298,   299,   300,   104,   104,    90,    37,   101,   101,   319,
     320,    91,   602,   629,   630,   631,   632,   633,   634,   635,
     636,   637,   638,   639,   640,   641,   642,   643,   644,   101,
      73,   455,   456,   457,   458,    37,   101,     9,    91,    73,
     104,   104,   605,    45,    46,    47,    48,    49,    50,    36,
      52,   101,   101,    55,    56,    57,   104,   101,    45,    46,
      47,    48,    49,    50,   375,    52,    91,    91,    55,    56,
      57,   101,   101,   101,   664,    91,    91,    70,   101,   695,
       1,    91,   101,   101,   101,   442,   448,    34,   466,   548,
     599,    27,    94,    95,    96,    97,    98,    99,   100,   101,
     363,    -1,    -1,    -1,   375,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   424,    -1,    -1,   427,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    48,    49,    50,
     440,    52,   443,   444,    55,    56,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      48,   461,    -1,    -1,    52,    -1,   466,    55,    56,    57,
     470,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    25,    94,    95,    96,    97,
      98,    99,   100,    33,    34,   629,   630,   631,   632,   633,
     634,   635,   636,   637,   638,   639,   640,   641,   642,   643,
     644,    -1,    -1,    -1,    -1,   567,    -1,    -1,    -1,    -1,
      -1,     1,    -1,    63,   544,    -1,    -1,    -1,   548,    -1,
     550,   551,   550,    -1,    -1,    75,    -1,    -1,    -1,    -1,
      -1,   561,    -1,    -1,    -1,    -1,   567,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   579,
      -1,   695,    -1,    -1,    -1,    45,    46,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,    -1,
     120,   121,   122,   123,    -1,    -1,    -1,    -1,     1,    -1,
     610,    -1,   610,    -1,   614,    -1,    -1,    -1,    -1,    -1,
      -1,   621,   142,   143,    -1,   145,   146,   147,   148,    -1,
     150,    -1,    -1,   153,    94,    95,    96,    97,    98,    99,
     100,   101,    -1,    -1,    -1,    -1,    -1,    -1,   649,    -1,
      -1,   171,    45,    46,    47,    48,    49,    50,    -1,    52,
      17,   181,    55,    56,    57,    -1,    -1,   667,    -1,    -1,
       1,   671,    -1,    -1,    -1,    -1,    -1,    -1,   678,    -1,
      37,   201,    -1,    -1,   204,    -1,    -1,   207,    45,    46,
      47,    48,    49,    50,    -1,    52,   697,    -1,    55,    56,
      57,    94,    95,    96,    97,    98,    99,   100,   101,   709,
      -1,   711,    -1,    -1,    45,    46,    47,    48,    49,    50,
      -1,    52,   722,   723,    55,    56,    57,    -1,   728,    -1,
      -1,    -1,     1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
     101,   116,   117,   118,   119,    -1,    45,    46,    47,    48,
      49,    50,    -1,    52,    -1,   305,    55,    56,    57,    -1,
     310,   136,   137,   138,   139,   140,   141,    -1,    -1,   144,
      -1,    -1,    -1,    -1,    -1,    -1,   151,   152,    -1,    -1,
      -1,    -1,    -1,    -1,   334,    -1,   336,    -1,   338,    -1,
     340,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,   101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   369,
     370,   371,   372,    -1,   374,   375,     1,    -1,   378,    -1,
      -1,    -1,    -1,    -1,   384,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    -1,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
      45,    46,    47,    48,    49,    50,    -1,    52,    -1,    -1,
      55,    56,    57,     1,    -1,    -1,    -1,   437,   438,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    48,    49,    50,
      -1,    52,    -1,    -1,    55,    56,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,   101,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
     490,    -1,   317,    94,    95,    96,    97,    98,    99,   100,
     101,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   333,    -1,
     335,    -1,   337,    -1,   339,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   522,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,    -1,    -1,     1,   362,   363,   364,
     365,     6,    -1,    -1,    -1,    10,    11,    12,    13,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    -1,    37,    38,    39,    40,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,   623,    -1,    90,    -1,    -1,    93,    -1,
     455,   456,   457,   458,    -1,    -1,   101,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   477,   478,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   489,    -1,    -1,    -1,    -1,    -1,
     495,    -1,   497,   673,    -1,    -1,   501,   502,    -1,     6,
      -1,    -1,    -1,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    -1,
      37,    38,    39,    40,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    31,   587,   588,   589,   590,    93,    37,   593,   594,
     595,   596,    -1,    -1,   101,    45,    46,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    55,    56,    57,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    -1,    -1,    -1,   629,   630,   631,   632,   633,   634,
     635,   636,   637,   638,   639,   640,   641,   642,   643,   644,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,    -1,    -1,    -1,    -1,   660,    -1,    94,    95,    96,
      97,    98,    99,   100,    -1,   102,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     695,    -1,    -1,     1,    -1,   700,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    -1,    -1,   101,    -1,   103,    -1,   105,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    -1,    -1,    -1,    -1,   103,    -1,   105,
      -1,   107,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,    -1,   105,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    -1,    -1,    93,    94,    95,    96,    97,    98,    99,
     100,    -1,   102,   103,    -1,   105,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,    -1,   105,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,    -1,   102,   103,    -1,   105,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,    -1,    -1,   103,
      -1,   105,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,
      -1,   103,    -1,   105,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    47,    -1,    -1,
      -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    -1,
      -1,    -1,    -1,   103,    -1,   105,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    -1,    -1,    -1,    -1,   103,   104,   105,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    -1,    -1,    -1,    -1,   103,   104,   105,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    -1,    -1,    -1,    -1,   103,
     104,   105,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,
      -1,   103,   104,   105,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    -1,
      -1,    -1,    -1,   103,   104,   105,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    -1,    -1,    -1,    -1,   103,    -1,   105,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    -1,    -1,    -1,    -1,   103,    -1,   105,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    -1,    -1,    -1,    -1,   103,
      -1,   105,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,
      -1,   103,    -1,   105,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    -1,
      -1,    -1,    -1,   103,    -1,   105,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    -1,    -1,    -1,    -1,   103,    -1,   105,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    -1,    -1,    -1,    -1,   103,    -1,   105,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    -1,    -1,    -1,    -1,   103,
      -1,   105,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,     6,   105,    -1,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    -1,    37,    38,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      -1,    -1,    -1,    -1,     6,    -1,    -1,   101,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    -1,    37,    38,    39,    40,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      82,    83,    84,    85,    86,    87,    -1,    -1,    90,    -1,
      -1,    93,    -1,    -1,    -1,    -1,     6,    -1,    -1,   101,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    -1,    -1,
      90,    -1,    -1,    93,    -1,    -1,    -1,    -1,     6,    -1,
      -1,   101,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    -1,    37,
      38,    39,    40,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      -1,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    -1,
      -1,     6,    -1,   101,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,     6,    -1,    93,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,     6,    -1,    93,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,     6,    -1,    93,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
       6,    -1,    -1,    93,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    -1,    25,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      -1,    37,    -1,    39,    40,    -1,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    -1,    75,
      76,    77,    78,    79,    31,    -1,    82,    83,    84,    85,
      37,    87,    -1,    -1,    -1,    -1,    -1,    93,    45,    46,
      47,    48,    49,    50,    -1,    52,    31,    -1,    55,    56,
      57,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    48,    49,    50,    -1,    52,    -1,    -1,
      55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    37,    94,    95,    96,
      97,    98,    99,   100,    45,    46,    47,    48,    49,    50,
      -1,    52,    31,    -1,    55,    56,    57,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,    45,    46,    47,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
      45,    46,    47,    48,    49,    50,    -1,    52,    31,    -1,
      55,    56,    57,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,    45,    46,    47,    48,    49,    50,    -1,    52,
      -1,    -1,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,    45,    46,    47,    48,
      49,    50,    -1,    52,    31,    -1,    55,    56,    57,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,    45,    46,
      -1,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    45,    46,    47,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    -1,    94,    95,    96,    97,    98,
      99,   100,    45,    46,    47,    48,    49,    50,    -1,    52,
      -1,    -1,    55,    56,    57,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,    -1,   102,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,    -1,    -1,
      -1,   104,    45,    46,    47,    48,    49,    50,    -1,    52,
      -1,    -1,    55,    56,    57,    45,    46,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    55,    56,    57,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,    -1,   102,
      -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,    99,
     100,    -1,   102,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,    -1,   102,    45,    46,    47,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    48,    49,    50,
      -1,    52,    -1,    -1,    55,    56,    57,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,   101,    84,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,    45,    46,    47,    48,    49,    50,    -1,
      52,    -1,    -1,    55,    56,    57,    45,    46,    -1,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    45,
      46,    -1,    48,    -1,    -1,    -1,    52,    -1,    -1,    55,
      56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    37,    38,
      39,    40,    41,    42,    43,    44,    53,    54,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    75,    76,    77,    78,    79,    82,
      83,    84,    85,    86,    87,    88,    90,    93,   101,   109,
     110,   111,   112,   113,   118,   120,   124,   125,   126,   127,
     131,   133,   134,   135,   137,   138,   139,   160,     9,    16,
      18,    19,    21,    22,    23,    24,    27,    28,    30,    33,
      34,    35,    36,    41,    42,    43,    44,    78,    86,    87,
      88,    89,    93,   119,   120,   121,    10,    11,    14,    15,
      58,    60,    62,    64,    78,   150,     1,    34,   101,   121,
     122,   123,     4,     5,     7,     8,    10,    11,    12,    13,
      14,    15,    20,    29,    51,    58,    60,    62,    64,    78,
      83,    97,    98,   103,   105,   121,   128,   130,   146,   147,
     150,   152,   154,   157,   158,   162,   165,   166,   167,   170,
     150,   103,   167,   121,     1,   101,   167,    34,   128,   167,
      39,    40,   121,   147,   149,   160,   101,   121,   128,   167,
     128,   130,   122,   121,   167,   122,   121,   101,   126,     0,
     111,    47,   128,   167,    47,   101,   102,   103,   153,   166,
     153,   166,   153,   166,   153,   167,   167,   167,   167,    72,
       1,   101,   101,     1,   101,     1,   101,   102,   153,   153,
     167,   167,   166,    66,    67,    68,    69,    70,    72,    77,
     121,   167,   166,   166,   104,   147,   167,     4,     5,     7,
       8,   107,   121,   163,   164,   167,   171,   172,   103,     1,
     101,     1,   101,    31,    45,    46,    47,    48,    49,    50,
      52,    55,    56,    57,    94,    95,    96,    97,    98,    99,
     100,   102,   129,   146,   168,   170,     1,   101,   167,    36,
     122,   101,     1,   101,     1,   101,     1,   101,    26,    53,
      54,   121,   167,    37,    47,   132,     1,   101,     1,   101,
      39,     1,   101,     1,   101,     1,   101,    31,     1,    37,
     101,     1,   101,   122,   167,     1,   101,   167,   121,   104,
     167,    31,    31,    31,    37,    31,    37,    31,    37,    31,
      37,    58,    60,    62,    64,   151,   101,   101,   101,   121,
      31,   168,    71,    79,    71,    79,    71,    79,    71,    79,
      71,    79,    31,    31,    84,    85,   168,   102,   104,   107,
     107,   107,   107,   106,   107,   102,   106,   106,   102,   104,
     147,   172,   101,   101,    29,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   172,    45,    46,    47,    48,    49,    50,    52,
      55,    56,    57,    94,    95,    96,    97,    98,    99,   100,
     102,   129,   101,     1,   101,   124,     1,   101,   101,   101,
     101,    78,   147,   148,   150,   148,   148,    25,    47,     1,
     101,   167,   101,     9,    89,   101,   114,   115,   116,   117,
     101,   167,   101,   101,   101,    73,    75,    82,    83,   161,
     166,   101,   126,   101,   124,   140,   141,   142,   101,     1,
     101,     1,   101,   101,     1,   101,   102,    15,    15,   166,
     167,   166,   167,   166,   167,   166,   167,    25,    31,   159,
      29,    59,    61,    63,    65,    73,    74,    75,    76,    80,
      81,    82,    83,   155,   156,   166,   155,   166,   166,   102,
     129,   104,   172,   167,   167,   167,   167,   167,   164,   167,
     167,   168,   102,   129,   167,   129,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   172,   101,   101,   125,   143,   144,   145,
      18,   101,   140,     1,   101,     1,   101,     1,   101,   167,
     167,   101,   140,     1,   101,   115,   119,   121,   117,   101,
     166,   169,   170,   169,   169,   169,   140,    92,   142,   101,
     140,   101,   101,   104,   172,   166,   166,    31,    31,    31,
      31,   166,   167,   159,   159,   159,   159,   166,   166,   159,
     166,   166,   172,   104,   129,   102,   129,   104,   129,   143,
      18,    90,   136,   145,   101,   124,   140,   101,   101,   101,
       1,   101,    17,    37,   140,    91,   101,   101,   122,    45,
      46,    48,    49,    50,    52,    55,    56,    57,    94,    95,
      96,    97,    98,    99,   100,    31,    92,   101,   140,     9,
     129,   166,   166,   166,   166,   166,   166,   166,   166,    31,
      73,   156,   129,   104,   172,   104,   136,   101,   124,   101,
     143,   101,   140,    37,   167,    91,   101,     1,   101,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,    73,   101,     9,   119,   104,
      73,   104,   129,   143,   136,   140,    91,   167,     1,   101,
     101,   101,   140,   169,   119,   101,   166,   104,   136,    91,
     101,     1,   101,   101,   140,   140,   101,   101,   101,   140,
     140,    91,   140,    91,    91,   101,    91,   101,   101,   101
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   108,   109,   110,   110,   111,   111,   111,   111,   111,
     111,   112,   112,   113,   113,   114,   114,   115,   115,   116,
     116,   117,   117,   117,   118,   118,   118,   118,   119,   119,
     119,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     122,   122,   122,   123,   123,   124,   124,   125,   125,   125,
     125,   125,   125,   126,   126,   126,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   129,   129,   130,   130,
     130,   130,   130,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   132,   132,   133,   133,   133,
     133,   133,   133,   134,   134,   134,   135,   135,   135,   135,
     136,   136,   137,   137,   137,   137,   137,   137,   137,   137,
     138,   138,   138,   139,   140,   140,   141,   141,   142,   142,
     143,   143,   144,   144,   145,   145,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   147,   148,   148,   149,   149,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   151,   151,   151,   151,   152,   152,   153,   153,   153,
     153,   154,   154,   154,   154,   154,   155,   155,   155,   155,
     155,   156,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   158,   158,   158,   158,   158,   158,   158,   158,
     159,   159,   160,   160,   161,   161,   161,   161,   161,   162,
     162,   162,   163,   163,   163,   164,   164,   164,   164,   164,
     165,   165,   166,   166,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,   167,   167,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     170,   170,   171,   171,   172,   172
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       3,     5,     6,     4,     5,     5,     6,     0,     1,     1,
       2,     1,     1,     3,     8,     9,     5,     6,     0,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     2,     1,     3,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     4,     3,     4,
       3,     4,     3,     4,     3,     4,     3,     4,     3,     4,
       5,     3,     4,     3,     4,     2,     3,     2,     3,     3,
       4,     3,     4,     3,     4,     2,     0,     2,     4,     3,
       5,     2,     4,     6,     5,     7,     0,     1,     2,     2,
       5,     4,     3,     5,     6,     5,     6,     5,     6,     5,
       6,     4,     5,     4,     5,     1,     1,     3,     4,     3,
       4,     3,     4,     4,     6,     7,     6,     8,     7,     9,
       0,     2,     7,     8,    11,    12,    12,    13,     9,    10,
       4,     6,     7,     2,     0,     1,     1,     2,     1,     1,
       0,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       2,     3,     5,     6,     8,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     5,     2,     5,
       2,     2,     4,     6,     4,     6,     4,     6,     4,     6,
       5,     1,     1,     1,     1,     2,     2,     1,     2,     4,
       6,     2,     4,     4,     1,     1,     1,     1,     5,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     6,     6,     6,     6,     6,     4,     4,     4,
       1,     1,     2,     4,     1,     2,     5,     2,     2,     3,
       3,     3,     1,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       4,     4,     0,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




/* The kind of the lookahead of this context.  */
static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx) YY_ATTRIBUTE_UNUSED;

static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx)
{
  return yyctx->yytoken;
}



/* User defined function to report a syntax error.  */
static int
yyreport_syntax_error (const yypcontext_t *yyctx);

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_tVARID: /* tVARID  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2478 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2484 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2490 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2496 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_script: /* script  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2502 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_scriptpart: /* scriptpart  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2508 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_macro: /* macro  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2514 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_factory: /* factory  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2520 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_method: /* method  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2526 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_methodlistline: /* methodlistline  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2532 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_handler: /* handler  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2538 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2544 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2550 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmt: /* stmt  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2556 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmt_insideif: /* stmt_insideif  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2562 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtoneliner: /* stmtoneliner  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2568 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_proc: /* proc  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2574 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_asgn: /* asgn  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2580 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_definevars: /* definevars  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2586 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ifstmt: /* ifstmt  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2592 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ifelsestmt: /* ifelsestmt  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2598 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_loop: /* loop  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2604 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tell: /* tell  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2610 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_when: /* when  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2616 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtlistline: /* stmtlistline  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2622 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtlistline_insideif: /* stmtlistline_insideif  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2628 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_simpleexpr_nounarymath: /* simpleexpr_nounarymath  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2634 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_var: /* var  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2640 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_varorchunk: /* varorchunk  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2646 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_varorthe: /* varorthe  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2652 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_chunk: /* chunk  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2658 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_object: /* object  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2664 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_the: /* the  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2670 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_theobj: /* theobj  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2676 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_menu: /* menu  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2682 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_thedatetime: /* thedatetime  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2688 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_thenumberof: /* thenumberof  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2694 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_writablethe: /* writablethe  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2700 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_writabletheobj: /* writabletheobj  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2706 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_list: /* list  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2712 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_proppair: /* proppair  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2718 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_unarymath: /* unarymath  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2724 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_simpleexpr: /* simpleexpr  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2730 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2736 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr_nounarymath: /* expr_nounarymath  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2742 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr_noeq: /* expr_noeq  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2748 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_sprite: /* sprite  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2754 "engines/director/lingo/lingo-gr.cpp"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* script: scriptpartlist  */
#line 225 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = Common::SharedPtr<Node>(new ScriptNode((yyvsp[0].nodelist))); (yyval.node) = nullptr; }
#line 3024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 227 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3035 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 233 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3045 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 240 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3051 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* scriptpart: tENDCLAUSE endargdef '\n'  */
#line 245 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 3057 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3063 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* macro: tMACRO ID idlist error '\n' stmtlist  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-4].s), (yyvsp[-3].idlist), (yyvsp[0].nodelist))); }
#line 3069 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* factory: tFACTORY ID '\n' methodlist  */
#line 280 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 3075 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* factory: tFACTORY ID error '\n' methodlist  */
#line 281 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new FactoryNode((yyvsp[-3].s), (yyvsp[0].nodelist))); }
#line 3081 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3087 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* method: tMETHOD ID idlist error '\n' stmtlist  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-4].s), (yyvsp[-3].idlist), (yyvsp[0].nodelist))); }
#line 3093 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlist: %empty  */
#line 288 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3099 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* nonemptymethodlist: methodlistline  */
#line 292 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3110 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3120 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* methodlistline: '\n'  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3126 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* methodlistline: tENDCLAUSE endargdef '\n'  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 3132 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 3141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* handler: tON ID idlist error '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                                                      {	// D3
		TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-7].s), (yyvsp[-6].idlist), (yyvsp[-3].nodelist)));
		checkEnd((yyvsp[-2].s), (yyvsp[-7].s), false);
		delete (yyvsp[-2].s); }
#line 3150 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* handler: tON ID idlist '\n' stmtlist  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3157 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* handler: tON ID idlist error '\n' stmtlist  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                            {	// D4. No 'end' clause
		TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-4].s), (yyvsp[-3].idlist), (yyvsp[0].nodelist))); }
#line 3164 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* endargdef: ID  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3170 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* endargdef: endargdef ',' ID  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3176 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tABBREVIATED  */
#line 336 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 3182 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tABBREV  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 3188 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tABBR  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 3194 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tAFTER  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 3200 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tBEFORE  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 3206 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tCAST  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 3212 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tCASTLIB  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("castLib"); }
#line 3218 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tCHAR  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 3224 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tCHARS  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 3230 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tDATE  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 3236 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tDELETE  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 3242 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tDOWN  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 3248 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tFIELD  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 3254 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tFRAME  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 3260 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tHILITE  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 3266 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tIN  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 3272 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tINTERSECTS  */
#line 352 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 3278 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tINTO  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 3284 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tITEM  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 3290 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tITEMS  */
#line 355 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 3296 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tLAST  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 3302 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tLINE  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 3308 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tLINES  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 3314 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tLONG  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 3320 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tMEMBER  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("member"); }
#line 3326 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tMENU  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 3332 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tMENUITEM  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 3338 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tMENUITEMS  */
#line 363 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 3344 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tMOVIE  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 3350 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tNEXT  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 3356 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tNUMBER  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 3362 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tOF  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 3368 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tPREVIOUS  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 3374 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tREPEAT  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 3380 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tSCRIPT  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 3386 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tASSERTERROR  */
#line 371 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 3392 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tSHORT  */
#line 372 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 3398 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tSOUND  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 3404 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tSPRITE  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 3410 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tTHE  */
#line 375 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 3416 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tTIME  */
#line 376 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 3422 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tTO  */
#line 377 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 3428 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWHILE  */
#line 378 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 3434 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* CMDID: tWINDOW  */
#line 379 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 3440 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* CMDID: tWITH  */
#line 380 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 3446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* CMDID: tWITHIN  */
#line 381 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 3452 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* CMDID: tWORD  */
#line 382 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 3458 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* CMDID: tWORDS  */
#line 383 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 3464 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tELSE  */
#line 387 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 3470 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tENDCLAUSE  */
#line 388 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); delete (yyvsp[0].s); }
#line 3476 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tEXIT  */
#line 389 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3482 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tFACTORY  */
#line 390 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3488 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tGLOBAL  */
#line 391 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3494 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tGO  */
#line 392 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3500 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tIF  */
#line 393 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3506 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tINSTANCE  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3512 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tMACRO  */
#line 395 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3518 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tMETHOD  */
#line 396 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3524 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tON  */
#line 397 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3530 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tOPEN  */
#line 398 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3536 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tPLAY  */
#line 399 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3542 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tPROPERTY  */
#line 400 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3548 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* ID: tPUT  */
#line 401 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3554 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* ID: tRETURN  */
#line 402 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("return"); }
#line 3560 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* ID: tSET  */
#line 403 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3566 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* ID: tTELL  */
#line 404 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3572 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* ID: tTHEN  */
#line 405 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3578 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* idlist: %empty  */
#line 408 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3584 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* nonemptyidlist: ID  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3593 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3601 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* stmt: tENDIF '\n'  */
#line 428 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3607 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: CMDID cmdargs '\n'  */
#line 444 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3613 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: CMDID cmdargs error '\n'  */
#line 445 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new CmdNode((yyvsp[-3].s), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3619 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tPUT cmdargs '\n'  */
#line 446 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3625 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tPUT cmdargs error '\n'  */
#line 447 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("put"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3631 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tGO cmdargs '\n'  */
#line 448 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3637 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tGO cmdargs error '\n'  */
#line 449 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("go"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3643 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proc: tGO frameargs '\n'  */
#line 450 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3649 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tGO frameargs error '\n'  */
#line 451 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("go"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3655 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tPLAY cmdargs '\n'  */
#line 452 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3661 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tPLAY cmdargs error '\n'  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("play"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3667 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* proc: tPLAY frameargs '\n'  */
#line 454 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3673 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* proc: tPLAY frameargs error '\n'  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("play"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3679 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* proc: tOPEN cmdargs '\n'  */
#line 456 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3685 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* proc: tOPEN cmdargs error '\n'  */
#line 457 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("open"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3691 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* proc: tOPEN expr tWITH expr '\n'  */
#line 458 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3701 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* proc: tNEXT tREPEAT '\n'  */
#line 463 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3707 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* proc: tNEXT tREPEAT error '\n'  */
#line 464 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new NextRepeatNode()); }
#line 3713 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* proc: tEXIT tREPEAT '\n'  */
#line 465 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3719 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* proc: tEXIT tREPEAT error '\n'  */
#line 466 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new ExitRepeatNode()); }
#line 3725 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* proc: tEXIT '\n'  */
#line 467 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3731 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* proc: tEXIT error '\n'  */
#line 468 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new ExitNode()); }
#line 3737 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* proc: tRETURN '\n'  */
#line 469 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode(nullptr); }
#line 3743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* proc: tRETURN error '\n'  */
#line 470 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new ReturnNode(nullptr)); }
#line 3749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* proc: tRETURN expr '\n'  */
#line 471 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode((yyvsp[-1].node)); }
#line 3755 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* proc: tRETURN expr error '\n'  */
#line 472 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new ReturnNode((yyvsp[-2].node))); }
#line 3761 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* proc: tDELETE chunk '\n'  */
#line 473 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* proc: tDELETE chunk error '\n'  */
#line 474 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new DeleteNode((yyvsp[-2].node))); }
#line 3773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* proc: tHILITE chunk '\n'  */
#line 475 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3779 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* proc: tHILITE chunk error '\n'  */
#line 476 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new HiliteNode((yyvsp[-2].node))); }
#line 3785 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* proc: tASSERTERROR stmtoneliner  */
#line 477 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* cmdargs: %empty  */
#line 480 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3799 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* cmdargs: expr trailingcomma  */
#line 483 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3809 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 488 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd arg, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3818 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 492 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3829 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 498 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* cmdargs: '(' ')'  */
#line 503 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3847 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* cmdargs: '(' expr ',' ')'  */
#line 506 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 511 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3866 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* cmdargs: '(' var expr_nounarymath trailingcomma ')'  */
#line 515 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3877 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* cmdargs: '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* frameargs: tFRAME expr  */
#line 534 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3897 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* frameargs: tMOVIE expr  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3908 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 545 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3919 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* frameargs: expr tOF tMOVIE expr  */
#line 551 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3930 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* frameargs: tFRAME expr expr_nounarymath  */
#line 557 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 565 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* asgn: tPUT expr tINTO varorchunk error '\n'  */
#line 566 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new PutIntoNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 567 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* asgn: tPUT expr tAFTER varorchunk error '\n'  */
#line 568 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new PutAfterNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 569 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* asgn: tPUT expr tBEFORE varorchunk error '\n'  */
#line 570 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new PutBeforeNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* asgn: tSET varorthe to expr '\n'  */
#line 571 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* asgn: tSET varorthe to expr error '\n'  */
#line 572 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new SetNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 171: /* asgn: CMDID tEQ expr '\n'  */
#line 575 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SetNode(new VarNode((yyvsp[-3].s)), (yyvsp[-1].node)); }
#line 3995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 172: /* asgn: CMDID tEQ expr error '\n'  */
#line 576 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new SetNode(new VarNode((yyvsp[-4].s)), (yyvsp[-2].node))); }
#line 4001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* asgn: writablethe tEQ expr '\n'  */
#line 577 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 4007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* asgn: writablethe tEQ expr error '\n'  */
#line 578 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new SetNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 4013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* definevars: tGLOBAL idlist '\n'  */
#line 583 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 4019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* definevars: tGLOBAL idlist error '\n'  */
#line 584 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new GlobalNode((yyvsp[-2].idlist))); }
#line 4025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* definevars: tPROPERTY idlist '\n'  */
#line 585 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 4031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* definevars: tPROPERTY idlist error '\n'  */
#line 586 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new PropertyNode((yyvsp[-2].idlist))); }
#line 4037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* definevars: tINSTANCE idlist '\n'  */
#line 587 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 4043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* definevars: tINSTANCE idlist error '\n'  */
#line 588 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new InstanceNode((yyvsp[-2].idlist))); }
#line 4049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* ifstmt: tIF expr tTHEN stmt  */
#line 591 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 4058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 595 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 4065 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* ifstmt: tIF expr tTHEN error '\n' stmtlist_insideif endif  */
#line 597 "engines/director/lingo/lingo-gr.y"
                                                            {
		TRIM_GARBAGE((yyval.node), new IfStmtNode((yyvsp[-5].node), (yyvsp[-1].nodelist))); }
#line 4072 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 601 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 4083 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 187: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 607 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 4092 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 611 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 4101 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 615 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 4108 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* endif: %empty  */
#line 619 "engines/director/lingo/lingo-gr.y"
                        {
		LingoCompiler *compiler = g_lingo->_compiler;
		warning("LingoCompiler::parse: no end if at line %d col %d in %s id: %d",
			compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
			compiler->_assemblyContext->_id);

		}
#line 4120 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 628 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4127 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* loop: tREPEAT tWHILE expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 630 "engines/director/lingo/lingo-gr.y"
                                                                  {
		TRIM_GARBAGE((yyval.node), new RepeatWhileNode((yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4134 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 194: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4141 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* loop: tREPEAT tWITH ID tEQ expr tTO expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		TRIM_GARBAGE((yyval.node), new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), false, (yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4148 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 636 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4155 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 638 "engines/director/lingo/lingo-gr.y"
                                                                                                   {
		TRIM_GARBAGE((yyval.node), new RepeatWithToNode((yyvsp[-10].s), (yyvsp[-8].node), true, (yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4162 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4169 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* loop: tREPEAT tWITH ID tIN expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 642 "engines/director/lingo/lingo-gr.y"
                                                                        {
		TRIM_GARBAGE((yyval.node), new RepeatWithInNode((yyvsp[-7].s), (yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4176 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* tell: tTELL expr tTO stmtoneliner  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 4185 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 201: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4192 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* tell: tTELL expr error '\n' stmtlist tENDTELL '\n'  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                        {
		TRIM_GARBAGE((yyval.node), new TellNode((yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4199 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* when: tWHEN '\n'  */
#line 656 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WhenNode((yyvsp[-1].w).eventName, (yyvsp[-1].w).stmt); }
#line 4205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* stmtlist: %empty  */
#line 658 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 4211 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* nonemptystmtlist: stmtlistline  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 4222 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 207: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 669 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 4232 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* stmtlistline: '\n'  */
#line 676 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 4238 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* stmtlist_insideif: %empty  */
#line 680 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 4244 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 685 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 4255 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 691 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 4265 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* stmtlistline_insideif: '\n'  */
#line 698 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 4271 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* simpleexpr_nounarymath: tINT  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 4277 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* simpleexpr_nounarymath: tFLOAT  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 4283 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* simpleexpr_nounarymath: tSYMBOL  */
#line 707 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 4289 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* simpleexpr_nounarymath: tSTRING  */
#line 708 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 4295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 709 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 4301 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 221: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 710 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 4307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 222: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 711 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 4313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 223: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath trailingcomma ')'  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.node) = new FuncNode((yyvsp[-5].s), args); }
#line 4324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 224: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 718 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.node) = new FuncNode((yyvsp[-7].s), (yyvsp[-2].nodelist)); }
#line 4334 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 225: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 723 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 4340 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 231: /* var: ID  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 4346 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* chunk: tFIELD refargs  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 4352 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* chunk: tCAST simpleexpr tOF tCASTLIB simpleexpr  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("cast"), args);	}
#line 4362 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* chunk: tCAST refargs  */
#line 747 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 4368 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* chunk: tMEMBER simpleexpr tOF tCASTLIB simpleexpr  */
#line 748 "engines/director/lingo/lingo-gr.y"
                                                           {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("member"), args);	}
#line 4378 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* chunk: tMEMBER refargs  */
#line 753 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("member"), (yyvsp[0].nodelist)); }
#line 4384 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* chunk: tCASTLIB refargs  */
#line 754 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("castLib"), (yyvsp[0].nodelist)); }
#line 4390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* chunk: tCHAR expr tOF simpleexpr  */
#line 755 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4397 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 757 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4404 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* chunk: tWORD expr tOF simpleexpr  */
#line 759 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4411 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 761 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4418 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* chunk: tITEM expr tOF simpleexpr  */
#line 763 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 765 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* chunk: tLINE expr tOF simpleexpr  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4439 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 769 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4446 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 771 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 4452 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* chunktype: tCHAR  */
#line 774 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 4458 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* chunktype: tWORD  */
#line 775 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 4464 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* chunktype: tITEM  */
#line 776 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 4470 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* chunktype: tLINE  */
#line 777 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 4476 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* object: tSCRIPT refargs  */
#line 780 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 4482 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* object: tWINDOW refargs  */
#line 781 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 4488 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* refargs: simpleexpr  */
#line 784 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 4498 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* refargs: '(' ')'  */
#line 789 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 4506 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* refargs: '(' expr ',' ')'  */
#line 792 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 4516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4525 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 261: /* the: tTHE ID  */
#line 803 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 4531 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 262: /* the: tTHE ID tOF theobj  */
#line 804 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 4537 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 263: /* the: tTHE tNUMBER tOF theobj  */
#line 805 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 4543 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 812 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4549 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* theobj: tSOUND simpleexpr  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4555 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 270: /* theobj: tSPRITE simpleexpr  */
#line 814 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4561 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* menu: tMENU simpleexpr  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4567 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4573 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4579 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* thedatetime: tTHE tABBREV tDATE  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4585 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* thedatetime: tTHE tABBREV tTIME  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4591 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* thedatetime: tTHE tABBR tDATE  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4597 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* thedatetime: tTHE tABBR tTIME  */
#line 824 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4603 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* thedatetime: tTHE tLONG tDATE  */
#line 825 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 4609 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* thedatetime: tTHE tLONG tTIME  */
#line 826 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 4615 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* thedatetime: tTHE tSHORT tDATE  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 4621 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* thedatetime: tTHE tSHORT tTIME  */
#line 828 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 4627 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 832 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 4633 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 833 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 4639 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 284: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 834 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 4645 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 835 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 4651 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 836 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 4657 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 287: /* thenumberof: tTHE tNUMBER tOF tMENUS  */
#line 837 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenus, nullptr); }
#line 4663 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 288: /* thenumberof: tTHE tNUMBER tOF tXTRAS  */
#line 838 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfXtras, nullptr); }
#line 4669 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 289: /* thenumberof: tTHE tNUMBER tOF tCASTLIBS  */
#line 839 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNumberOfNode(kNumberOfCastlibs, nullptr); }
#line 4675 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 292: /* writablethe: tTHE ID  */
#line 844 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 4681 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* writablethe: tTHE ID tOF writabletheobj  */
#line 845 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 4687 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* writabletheobj: tMENU expr_noeq  */
#line 849 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4693 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 850 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4699 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* writabletheobj: tSOUND expr_noeq  */
#line 851 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* writabletheobj: tSPRITE expr_noeq  */
#line 852 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* list: '[' exprlist ']'  */
#line 855 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 4717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* list: '[' ':' ']'  */
#line 856 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 4723 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* list: '[' proplist ']'  */
#line 857 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 4729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 302: /* proplist: proppair  */
#line 863 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4738 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 303: /* proplist: proplist ',' proppair  */
#line 867 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4746 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* proplist: proplist ',' expr  */
#line 870 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4754 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* proppair: tSYMBOL ':' expr  */
#line 875 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4760 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* proppair: ID ':' expr  */
#line 876 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4766 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* proppair: tSTRING ':' expr  */
#line 877 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4772 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 308: /* proppair: tINT ':' expr  */
#line 878 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new IntNode((yyvsp[-2].i)), (yyvsp[0].node)); }
#line 4778 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 309: /* proppair: tFLOAT ':' expr  */
#line 879 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new FloatNode((yyvsp[-2].f)), (yyvsp[0].node)); }
#line 4784 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 310: /* unarymath: '+' simpleexpr  */
#line 882 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4790 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 311: /* unarymath: '-' simpleexpr  */
#line 883 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4796 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr: expr '+' expr  */
#line 893 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4802 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr: expr '-' expr  */
#line 894 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4808 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr: expr '*' expr  */
#line 895 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4814 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr: expr '/' expr  */
#line 896 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4820 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* expr: expr tMOD expr  */
#line 897 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4826 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* expr: expr '>' expr  */
#line 898 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4832 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* expr: expr '<' expr  */
#line 899 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4838 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* expr: expr tEQ expr  */
#line 900 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4844 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* expr: expr tNEQ expr  */
#line 901 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4850 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* expr: expr tGE expr  */
#line 902 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4856 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 326: /* expr: expr tLE expr  */
#line 903 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4862 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 327: /* expr: expr tAND expr  */
#line 904 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4868 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 328: /* expr: expr tOR expr  */
#line 905 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4874 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 329: /* expr: expr '&' expr  */
#line 906 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4880 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 330: /* expr: expr tCONCAT expr  */
#line 907 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4886 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 331: /* expr: expr tCONTAINS expr  */
#line 908 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4892 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 332: /* expr: expr tSTARTS expr  */
#line 909 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4898 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 335: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 918 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 336: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 919 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4910 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 337: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 920 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4916 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 338: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 921 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4922 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 339: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 922 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4928 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 340: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 923 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4934 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 341: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 924 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4940 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 342: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 925 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4946 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 343: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 926 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4952 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 344: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 927 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4958 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 345: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 928 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4964 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 346: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 929 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4970 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 347: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 930 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4976 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 348: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 931 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4982 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 349: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 932 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4988 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 350: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 933 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4994 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 351: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 934 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5000 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 354: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 939 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5006 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 355: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 940 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5012 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 356: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 941 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5018 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 357: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 942 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5024 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 358: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 943 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5030 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 359: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 944 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5036 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 360: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 945 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5042 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 361: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 946 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5048 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 362: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 947 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5054 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 363: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 948 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5060 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 364: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 949 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5066 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 365: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 950 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5072 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 366: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 951 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5078 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 367: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 952 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5084 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 368: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 953 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5090 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 369: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 954 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5096 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 370: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 957 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 5102 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 371: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 958 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 5108 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 372: /* exprlist: %empty  */
#line 961 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 5114 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 374: /* nonemptyexprlist: expr  */
#line 965 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 5123 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 375: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 969 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 5131 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 5135 "engines/director/lingo/lingo-gr.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        if (yyreport_syntax_error (&yyctx) == 2)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 974 "engines/director/lingo/lingo-gr.y"


int yyreport_syntax_error(const yypcontext_t *ctx) {
	int res = 0;

	Common::String msg = "syntax error, ";

	// Report the unexpected token.
	yysymbol_kind_t lookahead = yypcontext_token(ctx);
	if (lookahead != YYSYMBOL_YYEMPTY)
		msg += Common::String::format("unexpected %s", yysymbol_name(lookahead));

	// Report the tokens expected at this point.
	enum { TOKENMAX = 10 };
	yysymbol_kind_t expected[TOKENMAX];

	int n = yypcontext_expected_tokens(ctx, expected, TOKENMAX);
	if (n < 0)
		// Forward errors to yyparse.
		res = n;
	else
		for (int i = 0; i < n; ++i)
			msg += Common::String::format("%s %s", i == 0 ? ": expected" : " or", yysymbol_name(expected[i]));

	yyerror(msg.c_str());

	return res;
}
