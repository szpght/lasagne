grammar Lasagne;



program : topLevelStatement* EOF;

topLevelStatement : functionBlock;

functionBlock : Def name paramList block;

block : OpenBrace statement* CloseBrace;

statement : assignStatement;

assignStatement : lvalue AssignOperator rvalue;

rvalue : name | literal;
lvalue : name;

literal : IntLiteral;

paramList : OpenParen 
    paramDecls
    CloseParen;

paramDecls : (name Colon type (Comma paramDecls)*)?;

type : name;

Def: 'def';

name : ID;



AssignOperator : '=';
IntLiteral : ('0'..'9')+;
Colon : ':';
OpenBrace : '{';
CloseBrace: '}';
OpenParen : '(';
CloseParen : ')';
Comma: ',';
ID : [a-zA-Z] [a-zA-Z0-9]*;
WS : [ \t\r\n]+ -> skip ;
