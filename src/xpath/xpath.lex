/*#############################################################
# This is a file to demonstrate how easily Lex can be used
# with nYacc
#############################################################*/

%option noyywrap

%{
#include <stdlib.h>
#include <string.h>

#include "xpath.tab.hpp"
#include "xpathimpl.h"

#define YY_DECL int yylex(void *yylval, XPathImpl *xp)

/*### Function table ###*/
/*This provides an easy mapping of names to tokens*/

typedef struct
{
    char *name;
    int  token;
}  KeywordEntry;

KeywordEntry keywords[] =
{
    /* Basic keywords */
    { "and",                    AND                    },
    { "div",                    DIV                    },
    { "mod",                    MOD                    },
    { "or",                     OR                     },
    { "pi",                     PI                     },

    /* Axis types */
    { "ancestor",               ANCESTOR               },
    { "ancestor-or-self",       ANCESTOR_OR_SELF       },
    { "attribute",              ATTRIBUTE              },
    { "child",                  CHILD                  },
    { "descendant",             DESCENDANT             },
    { "descendant-or-self",     DESCENDANT_OR_SELF     },
    { "following",              FOLLOWING              },
    { "following-sibling",      FOLLOWING_SIBLING      },
    { "namespace",              NAMESPACE              },
    { "parent",                 PARENT                 },
    { "preceding",              PRECEDING              },
    { "preceding-sibling",      PRECEDING_SIBLING      },
    { "self",                   SELF                   },

    /* Function names */
    { "last",                   LAST                   },
    { "position",               POSITION               },
    { "count",                  COUNT                  },
    { "id",                     ID                     },
    { "local-name",             LOCAL_NAME             },
    { "namespace-uri",          NAMESPACE_URI          },
    { "name",                   NAME                   },
    { "string",                 STRING                 },
    { "concat",                 CONCAT                 },
    { "starts-with",            STARTS_WITH            },
    { "contains",               CONTAINS               },
    { "substring-before",       SUBSTRING_BEFORE       },
    { "substring-after",        SUBSTRING_AFTER        },
    { "substring",              SUBSTRING              },
    { "string-length",          STRING_LENGTH          },
    { "normalize-space",        NORMALIZE_SPACE        },
    { "translate",              TRANSLATE              },
    { "boolean",                BOOLEAN                },
    { "not",                    NOT                    },
    { "true",                   TRUE                   },
    { "false",                  FALSE                  },
    { "lang",                   LANG                   },
    { "number",                 NUMBER                 },
    { "sum",                    SUM                    },
    { "floor",                  FLOOR                  },
    { "ceiling",                CEILING                },
    { "round",                  ROUND                  },

    /* For NodeTest */
    { "processing-instruction", PROCESSING_INSTRUCTION },

    { NULL,                     0                      }
};


%}

ALPHA         [A-Za-z_]
DIGIT         [0-9]
ALPHA_NUMERIC {ALPHA}|{DIGIT}
IDENT         {ALPHA}({ALPHA_NUMERIC})*  
NUMBER        ({DIGIT})+
WHITE_SPACE   ([\ \n\r\t\f])+
FLOAT         -?({DIGIT})+|({DIGIT})*\.({DIGIT})+

%%

{FLOAT} { 
    char *endPtr;
    xp->numval = strtod(yytext, &endPtr);
    return NUMBER;
}

{WHITE_SPACE} { }

"\$"({IDENT}"\:")?{IDENT} {
    xp->strval = yytext;
    return DOLLAR_QNAME;
}

({IDENT}"\:")?{IDENT} {
    xp->strval = yytext;
    return QNAME;
}


{IDENT} {

    KeywordEntry *entry;
    for (entry=keywords; entry->name ; entry++)
        {
        if (strcmp(yytext, entry->name)==0)
            return entry->token;
        }
}


"\"" {
  xp->strval = "";
  int end_str = 0;
  int c1 = yyinput();
  int c2 = c1;
  while (!end_str)
    {
    if (c2 != '"' && c1 == '\"')
      end_str = 1;
    xp->strval += c1;
    c2 = c1;
    c1 = yyinput();
    }
  return LITERAL;
}

"<!--" {
  int end_str = 0;
  int c1 = yyinput();
  int c2 = 0;
  int c3 = 0;
  while (!end_str)
    {
    if (c3 == '-' && c2 != '-' && c1=='>')
      end_str = 1;
    c3 = c2; c2 = c1;
    c1 = yyinput();
    }
  return COMMENT;
}


"/" {
    return SLASH;
}

"//" {
    return SLASH_SLASH;
}

"\." {
    return DOT;
}

"\.\." {
    return DOT_DOT;
}



. {
    return yytext[0];
}

%%



void lexSetup(char *buf)
{
    yy_scan_string((const char *)buf);
}





