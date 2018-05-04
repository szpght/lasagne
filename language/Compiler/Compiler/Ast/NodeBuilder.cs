using System.Linq;
using Antlr4.Runtime;

namespace Lasagne.Compiler.Ast
{
    public class NodeBuilder
    {
        public T Build<T>(ParserRuleContext context) where T : Node, new()
        {
            var node = new T
            {
                StartLine = context.Start.Line,
                StartPosition = context.Start.Column,
                StopLine = context.Stop.Line,
                StopPosition = context.Stop.Column
            };

            if (node is INamedNode namedNode)
            {
                var contextMethods = context
                    .GetType()
                    .GetMethods();
                if (contextMethods.Any(m => m.Name == "identifier"))
                {
                    dynamic dynamicContext = context;
                    var name = dynamicContext.identifier().GetText();
                    namedNode.Name = name;
                }
            }

            return node;
        }
    }
}
