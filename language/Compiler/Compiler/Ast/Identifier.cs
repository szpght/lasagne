namespace Lasagne.Compiler.Ast
{
    public class Identifier : Expression, INamedNode
    {
        public string Name { get; set; }
    }
}
