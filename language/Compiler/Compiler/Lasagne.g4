grammar Lasagne;


program
    : topLevelStatement* EOF
    ;

topLevelStatement
    : functionBlock
    | enumBlock
    | structBlock
    | implementationBlock
    ;

enumBlock
    : Enum identifier OpenBrace enumOption* CloseBrace
    ;

enumOption
    : identifier
    | identifier Colon IntLiteral
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
    : Impl identifier OpenBrace methodDefinition* CloseBrace
    ;

methodDefinition
    : Pub? functionBlock
    ;

functionBlock
    : Def identifier paramList codeBlock
    ;

codeBlock
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
    | variableInlineDeclaration
    ;

literal
    : IntLiteral
    ;

variableInlineDeclaration
    : immutableVariableDeclaration
    | mutableVariableDeclaration
    ;

immutableVariableDeclaration
    : Let identifier
    ;

mutableVariableDeclaration
    : Var identifier
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
Impl: 'impl';
Let : 'let';
Pub : 'pub';
Var : 'var';
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
