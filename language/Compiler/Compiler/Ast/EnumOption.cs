namespace Lasagne.Compiler.Ast
{
    public class EnumOption : Node, INamedNode
    {
        public string Name { get; set; }
        public long? Value { get; set; }
    }
}
