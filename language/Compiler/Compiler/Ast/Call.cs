using System.Collections.Generic;

namespace Lasagne.Compiler.Ast
{
    public class Call : Expression
    {
        public Expression Target { get; set; }
        public List<Expression> Arguments { get; set; } = new List<Expression>();

        public void AddArguments(IEnumerable<Expression> arguments)
        {
            Arguments.AddRange(arguments);
        }
    }
}
