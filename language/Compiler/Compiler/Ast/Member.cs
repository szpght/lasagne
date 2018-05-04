namespace Lasagne.Compiler.Ast
{
    public class Member : Expression
    {
        public Expression Target { get; set; }
        public string Subject { get; set; }
    }
}
