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
    : typedParameter*
    ;

typedParameter
    : identifier Colon type
    ;

implementationBlock
    : Impl identifier OpenBrace functionBlock* CloseBrace
    ;

functionBlock
    : Def functionSignature codeBlock
    ;

functionSignature
    : Pub? identifier OpenParen paramDecls CloseParen (Colon type)
    ;

paramDecls
    : (typedParameter (Comma typedParameter)*)?
    ;

codeBlock
    : OpenBrace (statement Semicolon)* CloseBrace
    ;

statement
    : assignment
    | expression
    ;

expression
    : OpenParen expression CloseParen
    | expression multiplicationOperator expression
    | expression additionOperator expression
    | call
    | identifier
    | literal
    ;

assignment
    : valueAssignment
    | valueDefinition
    ;

valueAssignment
    : identifier AssignOperator expression
    ;

valueDefinition
    : variableDefinition
    | constantDefinition
    ;

variableDefinition
    : Var valueAssignment
    ;

constantDefinition
    : Let valueAssignment
    ;

literal
    : intLiteral
    ;

intLiteral
    : IntLiteral
    ;

call
    : callable OpenParen callArguments CloseParen
    ;

callable
    : identifier
    | member
    ;

member
    : identifier Dot identifier
    | member Dot identifier
    ;

callArguments
    : (expression (Comma expression)*)?
    ;

multiplicationOperator
    : Multiply
    | Divide
    | Modulo
    ;

additionOperator
    : Plus
    | Minus
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
Struct : 'struct';
Var : 'var';
AssignOperator : '=';
Plus : '+';
Minus : '-';
Multiply : '*';
Divide : '/';
Modulo : '%';
IntLiteral : ('0'..'9')+;
Colon : ':';
Semicolon : ';';
OpenBrace : '{';
CloseBrace: '}';
OpenParen : '(';
CloseParen : ')';
Comma : ',';
Dot : '.';
Identifier : [a-zA-Z_] [a-zA-Z0-9_]*;
WS : [ \t\r\n]+ -> skip ;
