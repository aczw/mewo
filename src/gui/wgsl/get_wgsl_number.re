#include "get_wgsl_number.hpp"

namespace mewo::language {

TextEditor::Iterator get_wgsl_number(TextEditor::Iterator start, TextEditor::Iterator end) {
  TextEditor::Iterator i = start;
  TextEditor::Iterator marker;
  
  /*!re2c
    re2c:api = custom;
	  re2c:api:style = free-form;
    re2c:define:YYCTYPE = ImWchar;
    re2c:define:YYPEEK = "i < end ? *i : 0";
	  re2c:define:YYSKIP = "++i;";
	  re2c:define:YYBACKUP = "marker = i;";
	  re2c:define:YYRESTORE = "i = marker;";
	  re2c:define:YYLESSTHAN = "i >= end";
    re2c:yyfill:enable = 0;

    digit      = [0-9];
    hexdigit   = [0-9a-fA-F];
    dec_exp    = [eE] [+-]? digit+;
    hex_exp    = [pP] [+-]? digit+;

    dec_int    = "0" | [1-9] digit*;
    dec_float  =
          digit+ "." digit* dec_exp? [fh]?
        | "." digit+ dec_exp? [fh]?
        | digit+ dec_exp [fh]?
        | digit+ [fh];

    hex_prefix = "0" [xX];
    hex_int    = hex_prefix hexdigit+ [iu]?;
    hex_float  =
          hex_prefix (
                hexdigit+ "." hexdigit* hex_exp? [fh]?
              | "." hexdigit+ hex_exp? [fh]?
              | hexdigit+ hex_exp [fh]?
          );

    wgsl_number = hex_float | hex_int | dec_float | dec_int [iu]?;

    wgsl_number { return i; }
    *           { return start; }
  */
}

}  // namespace mewo::language
