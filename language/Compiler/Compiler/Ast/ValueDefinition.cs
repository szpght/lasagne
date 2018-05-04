namespace Lasagne.Compiler.Ast
{
    public class ValueDefinition : Node
    {
        public bool IsConstant { get; set; }
        public ValueAssignment Assignment { get; set; }
    }
}
