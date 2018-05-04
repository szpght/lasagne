namespace Lasagne.Compiler.Ast
{
    public class ValueAssignment : Node, INamedNode
    {
        public string Name { get; set; }
        public Expression Value { get; set; }
    }
}
