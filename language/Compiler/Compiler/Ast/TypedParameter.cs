namespace Lasagne.Compiler.Ast
{
    public class TypedParameter : Node, INamedNode
    {
        public string Name { get; set; }
        public TypeNode Type { get; set; }
    }
}
