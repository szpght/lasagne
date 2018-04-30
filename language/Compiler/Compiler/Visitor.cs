using System;
using Antlr4.Runtime.Tree;
using Lasagne.Compiler.Ast;

namespace Lasagne.Compiler
{
    public class Visitor : LasagneBaseVisitor<Node>
    {
        private readonly NodeBuilder _nodeBuilder;

        public Visitor(NodeBuilder nodeBuilder)
        {
            _nodeBuilder = nodeBuilder;
        }

        public override Node VisitProgram(LasagneParser.ProgramContext context)
        {
            var node = _nodeBuilder.Build<CompilationUnit>(context);
            foreach (var child in context.children)
            {
                var subNode = Visit(child);
                if (subNode is null)
                {
                    Console.WriteLine($"Warning: omitted unimplemented construct {child.GetChild(0)?.GetType().Name}");
                    continue;
                }

                node.Children.Add(subNode);
            }

            return node;
        }

        public override Node VisitEnumBlock(LasagneParser.EnumBlockContext context)
        {
            var node = _nodeBuilder.Build<EnumDeclaration>(context);
            node.Name = context.identifier().GetText();

            foreach (var option in context.enumOption())
            {
                var value = Visit(option);
                node.Children.Add(value);
            }

            return node;
        }

        public override Node VisitEnumOption(LasagneParser.EnumOptionContext context)
        {
            var node = _nodeBuilder.Build<EnumOption>(context);
            node.Name = context.identifier().GetText();
            var intLiteral = context.IntLiteral();
            if (intLiteral is IParseTree value)
            {
                node.Value = long.Parse(value.GetText());
            }

            return node;
        }
    }
}
