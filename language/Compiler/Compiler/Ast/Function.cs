namespace Lasagne.Compiler.Ast
{
    public class Function : Node
    {
        public FunctionSignature Signature { get; set; }
        public CodeBlock Body { get; set; }
    }
}
