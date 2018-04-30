using Antlr4.Runtime;

namespace Lasagne.Compiler.Ast
{
    public class NodeBuilder
    {
        public T Build<T>(ParserRuleContext context) where T : Node, new()
        {
            return new T
            {
                StartLine = context.Start.Line,
                StartPosition = context.Start.Column,
                StopLine = context.Stop.Line,
                StopPosition = context.Stop.Column
            };
        }
    }
}
