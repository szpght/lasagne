namespace Lasagne.Compiler.Ast
{
    public class BinaryOperatorExpression : Expression
    {
        public Expression Left { get; set; }
        public Expression Right { get; set; }
        public BinaryOperator Operator { get; set; }
    }
}
