grammar Lasagne;


program
    :   topLevelStatement* EOF
    ;

topLevelStatement
    : functionBlock
    | enumBlock
    | structBlock
    | implementationBlock
    ;

enumBlock
    : Enum identifier OpenBrace enumOptions CloseBrace
    ;

enumOptions
    : identifier*
    ;

structBlock
    : Struct identifier OpenBrace structMembers CloseBrace
    ;

structMembers
    : structMember*
    ;

structMember
    : identifier Colon type
    ;

implementationBlock
    : OpenBrace methodDefinition* CloseBrace
    ;

methodDefinition
    : Pub? functionBlock
    ;

functionBlock
    : Def identifier paramList block
    ;

block
    : OpenBrace statement* CloseBrace
    ;

statement
    : assignStatement
    ;

assignStatement
    : lvalue AssignOperator rvalue
    ;

rvalue
    : identifier
    | literal
    ;

lvalue
    : identifier
    ;

literal
    : IntLiteral
    ;

paramList
    : OpenParen paramDecls CloseParen
    ;

paramDecls
    : (identifier Colon type (Comma paramDecls)*)?
    ;

type
    : identifier
    ;

identifier
    : Identifier
    ;

Def: 'def';
Enum : 'enum';
Pub : 'pub';
AssignOperator : '=';
IntLiteral : ('0'..'9')+;
Colon : ':';
OpenBrace : '{';
CloseBrace: '}';
OpenParen : '(';
CloseParen : ')';
Comma: ',';
Struct: 'struct';
Identifier : [a-zA-Z_] [a-zA-Z0-9_]*;
WS : [ \t\r\n]+ -> skip ;
