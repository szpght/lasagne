namespace Lasagne.Compiler.Ast
{
    public class StructMember : Node
    {
        public string Name { get; set; }
        public TypeNode Type { get; set; }
    }
}
