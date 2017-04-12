#!/usr/bin/env python
from __future__ import print_function
import sys
import os
import re
from collections import namedtuple
import peglet


Var = namedtuple('Var', ['name'])
StmtCmp = namedtuple('StmtCmp', ['left', 'op', 'right'])
SubBegin = namedtuple('SubBegin', ['name', 'args'])
SubEnd = namedtuple('SubEnd', [])
StmtAssign = namedtuple('StmtAssign', ['left', 'right'])
StmtElif = namedtuple('StmtElif', ['cond', 'body'])
StmtElse = namedtuple('StmtElse', ['body'])
StmtIf = namedtuple('StmtIf', ['cond', 'body', 'extra'])
StmtWhile = namedtuple('StmtWhile', ['cond', 'body'])
Label = namedtuple('Label', ['name'])
Line = namedtuple('Line', ['file', 'idx'])
Sub = namedtuple('Sub', ['name', 'args', 'body'])
Call = namedtuple('Call', ['name', 'args'])


class Parser(object):

    grammar = r"""
    wrapper     = ^ program $
    program     = statement program | statement 
    statement   = space sub space

    _           = [\t\x20]*
    space       = \s* | comments
    newline     = [\r\n]+
    comment     = \# [^\r\n$]+ newline
    comments    = _ newline comments
                | _ newline _
                | _ comment comments
                | _ comment _
    split       = comments
                | comments split
                | _ ; _ split
                | _ ; _
    
    name        = ([a-zA-Z_][_a-zA-Z0-9]*\b)
    label       = : name                            make_label
    maybe_label = space label space
                | space
    line        = maybe_label stmt split
    lines       = line lines
                | line    

    stmt        = var_assign | stmt_if | stmt_while | stmt_call
    var_assign  = var _ \= _ value                  make_assign

    sub         = begin lines end                   make_sub
    begin       = (\.begin\b) _ name_args split     clean_sub
    end         = (\.end\b)                         make_endsub
    
    name_args   = name sub_args                     make_subbegin
    sub_args    = _ var _ sub_args
                | _ var
                | _

    stmt_call   = name call_types                   make_call
    call_types  = _ label | call_args
    call_args   = _ value _ call_args  
                | _ value
                | _

    stmt_while  = (\.while\b) space bool split lines space (\.end\b)    make_while

    if_body     = space bool split lines space
    stmt_else   = (\.else\b) space lines space
    stmts_else  = stmt_else                         make_else
                | space
    stmts_elif  = stmt_elif stmts_elif
                | stmt_elif
                | space
    stmt_elif   = (\.elif\b) if_body                make_elif
    if_tail     = stmts_elif stmts_else             listify
    begin_if    = (\.if\b) if_body                  make_if_begin
    end_if      = if_tail (\.end\b)                 
    stmt_if     = begin_if end_if                   make_if
    bool        = value _ opcmp _ value             make_cmp
    opcmp       = (==|!=)

    value       = var | \( space stmt_call space \)
    var         = \$ name                           token_var
    """

    def __init__(self):
        kwargs = {"make_endsub": lambda *args: SubEnd(),
                  "make_cmp": StmtCmp,
                  "make_assign": StmtAssign,
                  "make_label": Label,
                  "make_while": lambda *x: StmtWhile(x[1], x[2:-1]),
                  "make_else": lambda *x: StmtElse(x[1:]),
                  "make_if": lambda *x: StmtIf(x[0][0], x[0][1], x[1]),
                  "make_if_begin": lambda *x: (x[1], x[2:]),
                  "make_elif": lambda *x: StmtElif(x[1], x[2:]),
                  "make_call": lambda *x: Call(x[0], x[1:]),
                  "make_sub": lambda *x: Sub(x[0].name, x[0].args, x[1:-1]),
                  "make_subbegin": lambda *args: SubBegin(args[0], args[1:]),
                  "clean_sub": lambda x, y: y,
                  "token_var": Var,
                  "hug"    : peglet.hug,
                  "listify": lambda *x: list(x),
                }
        self.parser = peglet.Parser(self.grammar, **kwargs)
    
    def __call__(self, program):
        return self.parser(program)


def str_expr(expr, inside=False):
    result = "<UHNKNOWN! " + str(expr) + " >"
    if isinstance(expr, Var):
        result = "$" + expr.name
    else:
        if isinstance(expr, StmtCmp):
            result = "%s %s %s" % (str_expr(expr.left, True), expr.op, str_expr(expr.right, True))
        elif isinstance(expr, Call):
            result = "%s %s" % (expr.name, str_expr(expr.args))
        elif isinstance(expr, (list, tuple)):
            result = " ".join([str_expr(X) for X in expr])
        if inside:
            result = "(" + result + ")"
    return result


def print_token(token, level=0):
    indent = (" " * level * 4)
    if isinstance(token, Sub):
        print(indent + ".begin %s %s" % (
            token.name,
            ' '.join([
                '$' + arg.name
                for arg in token.args])))
        for subtoken in token.body:
            print_token(subtoken, level + 1)
        print(indent + ".end")
    elif isinstance(token, Label):
        print(indent + ":" + token.name)
    elif isinstance(token, Call):
        print(indent + token.name, str_expr(token.args))
    elif isinstance(token, StmtIf):
        print(indent + ".if", str_expr(token.cond))
        for subtoken in token.body:
            print_token(subtoken, level + 1)
        for substmt in token.extra:
            print_token(substmt, level)
        print(indent + ".end")
    elif isinstance(token, StmtElif):
        print(indent + ".elif", str_expr(token.cond))
        for subtoken in token.body:
            print_token(subtoken, level + 1)
    elif isinstance(token, StmtElse):
        print(indent + ".else")
        for subtoken in token.body:
            print_token(subtoken, level + 1)
    elif isinstance(token, StmtWhile):
        print(indent + ".while", str_expr(token.cond))
        for subtoken in token.body:
            print_token(subtoken, level + 1)
        print(indent + ".end")
    elif isinstance(token, StmtAssign):
        print(indent + str_expr(token.left), "=", str_expr(token.right))
    else:
        print(indent, "Derp", token)


class Driver(object):
    def tokenize_file(self, handle):
        parser = Parser()        
        token_list = parser(handle.read())
        for token in token_list:
            print_token(token)
            #print(token,"\n")
        return token_list


if __name__ == "__main__":
    compiler = Driver()
    for filepath in sys.argv[1:]:
        with open(filepath, 'r') as handle:
            compiler.tokenize_file(handle)
