using System;
using System.Collections.Generic;
using System.Linq;
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

            var children = VisitMany(context.children);
            children = children.Where(child => child != null);
            node.AddChildren(children);

            return node;
        }

        public override Node VisitEnumBlock(LasagneParser.EnumBlockContext context)
        {
            var node = _nodeBuilder.Build<EnumDeclaration>(context);
            node.Name = context.identifier().GetText();
            var options = VisitMany(context.enumOption());
            node.AddChildren(options);

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

        public override Node VisitStructBlock(LasagneParser.StructBlockContext context)
        {
            var node = _nodeBuilder.Build<StructDeclaration>(context);
            node.Name = context.identifier().GetText();

            var members = VisitMany(context.structMembers().children);
            node.AddChildren(members);

            return node;
        }

        public override Node VisitStructMember(LasagneParser.StructMemberContext context)
        {
            var node = _nodeBuilder.Build<StructMember>(context);
            node.Name = context.identifier().GetText();
            node.Type = (TypeNode)Visit(context.type());
            return node;
        }

        private IEnumerable<Node> VisitMany(IEnumerable<IParseTree> tree)
        {
            if (tree is null)
            {
                return new List<Node>();
            }

            return tree.Select(Visit)
                .ToList();
        }
    }
}
