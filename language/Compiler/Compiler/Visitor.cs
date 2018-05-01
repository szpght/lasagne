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

        public override Node VisitTypedParameter(LasagneParser.TypedParameterContext context)
        {
            var node = _nodeBuilder.Build<TypedParameter>(context);
            node.Name = context.identifier().GetText();
            node.Type = (TypeNode)Visit(context.type());
            return node;
        }

        public override Node VisitFunctionBlock(LasagneParser.FunctionBlockContext context)
        {
            var node = _nodeBuilder.Build<Function>(context);
            node.Signature = (FunctionSignature) Visit(context.functionSignature());
            node.Body = (CodeBlock) Visit(context.codeBlock());
            return node;
        }

        public override Node VisitFunctionSignature(LasagneParser.FunctionSignatureContext context)
        {
            var node = _nodeBuilder.Build<FunctionSignature>(context);
            node.Name = context.identifier().GetText();
            var type = context.type();
            if (type != null)
            {
                node.ReturnedType = (TypeNode) Visit(type);
            }

            if (context.Pub() != null)
            {
                node.IsPublic = true;
            }

            var parameters = VisitMany<TypedParameter>(context.paramDecls().typedParameter());
            node.AddParameters(parameters);
            return node;
        }

        public override Node VisitCodeBlock(LasagneParser.CodeBlockContext context)
        {
            var node = _nodeBuilder.Build<CodeBlock>(context);
            var children = VisitMany(context.children);
            node.AddChildren(children);
            return node;
        }

        public override Node VisitType(LasagneParser.TypeContext context)
        {
            var node = _nodeBuilder.Build<TypeNode>(context);
            node.Name = context.identifier().GetText();
            return node;
        }

        public override Node VisitImplementationBlock(LasagneParser.ImplementationBlockContext context)
        {
            var node = _nodeBuilder.Build<Implementation>(context);
            node.Name = context.identifier().GetText();
            var methods = VisitMany<Function>(context.functionBlock());
            node.AddMethods(methods);
            return node;
        }

        private IEnumerable<Node> VisitMany(IEnumerable<IParseTree> tree)
        {
            return VisitMany<Node>(tree);
        }

        private IEnumerable<T> VisitMany<T>(IEnumerable<IParseTree> tree) where T: Node
        {
            if (tree is null)
            {
                return new List<T>();
            }

            return tree.Select(leaf => (T) Visit(leaf))
                .ToList();
        }
    }
}
