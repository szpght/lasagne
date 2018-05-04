using System;
using System.Collections.Generic;
using System.Linq;
using Antlr4.Runtime;
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
            var options = VisitMany(context.enumOption());
            node.AddChildren(options);

            return node;
        }

        public override Node VisitEnumOption(LasagneParser.EnumOptionContext context)
        {
            var node = _nodeBuilder.Build<EnumOption>(context);
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

            var members = VisitMany(context.structMembers().children);
            node.AddChildren(members);

            return node;
        }

        public override Node VisitTypedParameter(LasagneParser.TypedParameterContext context)
        {
            var node = _nodeBuilder.Build<TypedParameter>(context);
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
            var children = VisitMany(context.children)
                .Where(x => x != null); // remove terminal nodes
            node.AddChildren(children);
            return node;
        }

        public override Node VisitType(LasagneParser.TypeContext context)
        {
            var node = _nodeBuilder.Build<TypeNode>(context);
            return node;
        }

        public override Node VisitImplementationBlock(LasagneParser.ImplementationBlockContext context)
        {
            var node = _nodeBuilder.Build<Implementation>(context);
            var methods = VisitMany<Function>(context.functionBlock());
            node.AddMethods(methods);
            return node;
        }

        public override Node VisitValueAssignment(LasagneParser.ValueAssignmentContext context)
        {
            var node = _nodeBuilder.Build<ValueAssignment>(context);
            node.Value = (Expression) Visit(context.expression());
            return node;
        }

        public override Node VisitVariableDefinition(LasagneParser.VariableDefinitionContext context)
        {
            var node = _nodeBuilder.Build<ValueDefinition>(context);
            node.Assignment = (ValueAssignment)Visit(context.valueAssignment());
            node.IsConstant = false;
            return node;
        }

        public override Node VisitConstantDefinition(LasagneParser.ConstantDefinitionContext context)
        {
            var node = _nodeBuilder.Build<ValueDefinition>(context);
            node.Assignment = (ValueAssignment)Visit(context.valueAssignment());
            node.IsConstant = true;
            return node;
        }

        public override Node VisitExpression(LasagneParser.ExpressionContext context)
        {
            var children = VisitMany(context.children)
                .Where(x => x != null) // remove terminal nodes
                .ToList();

            switch (children.Count)
            {
                case 1:
                    {
                        var child = children.Single();
                        if (child is Expression)
                        {
                            return child;
                        }

                        var node = _nodeBuilder.Build<Expression>(context);
                        node.Child = child;
                        return node;
                    }
                case 3:
                    {
                        var node = _nodeBuilder.Build<BinaryOperatorExpression>(context);
                        node.Left = (Expression)children.First();
                        node.Operator = (BinaryOperator)children[1];
                        node.Right = (Expression)children.Last();
                        return node;
                    }

                default:
                    throw new Exception("shouldn't get here");
            }
        }

        public override Node VisitAdditionOperator(LasagneParser.AdditionOperatorContext context)
        {
            return ManuallyVisitBinaryOperator(context);
        }

        public override Node VisitMultiplicationOperator(LasagneParser.MultiplicationOperatorContext context)
        {
            return ManuallyVisitBinaryOperator(context);
        }

        private Node ManuallyVisitBinaryOperator(ParserRuleContext context)
        {
            var node = _nodeBuilder.Build<BinaryOperator>(context);
            node.Operator = context.GetText();
            return node;
        }

        public override Node VisitIntLiteral(LasagneParser.IntLiteralContext context)
        {
            var node = _nodeBuilder.Build<IntLiteral>(context);
            var valueString = context.IntLiteral().GetText();
            node.Value = long.Parse(valueString);
            return node;
        }

        public override Node VisitIdentifier(LasagneParser.IdentifierContext context)
        {
            var node = _nodeBuilder.Build<Identifier>(context);
            node.Name = context.GetText();
            return node;
        }

        public override Node VisitCall(LasagneParser.CallContext context)
        {
            var node = _nodeBuilder.Build<Call>(context);
            node.Target = (Expression) Visit(context.callable());
            var argumentsNode = Visit(context.callArguments());
            var arguments = argumentsNode.Children
                .Select(a => (Expression)a);
            node.AddArguments(arguments);
            return node;
        }

        public override Node VisitCallArguments(LasagneParser.CallArgumentsContext context)
        {
            var node = new AggregateNode();
            var arguments = VisitMany<Expression>(context.expression());
            node.AddChildren(arguments);
            return node;
        }

        public override Node VisitMember(LasagneParser.MemberContext context)
        {
            var node = _nodeBuilder.Build<Member>(context);

            var identifiers = context.identifier();
            node.Subject = identifiers.Last().GetText();

            switch (identifiers.Length)
            {
                case 1:
                    node.Target = (Expression)Visit(context.member());
                    break;
                case 2:
                    node.Target = (Expression)Visit(identifiers.First());
                    break;
            }

            return node;
        }

        // if visiting more than one non-null nodes, wrap them in AggregateNode
        protected override Node AggregateResult(Node aggregate, Node nextResult)
        {
            if (nextResult is null)
            {
                return aggregate;
            }

            if (aggregate is null)
            {
                return nextResult;
            }

            if (aggregate is AggregateNode)
            {
                aggregate.Children.Add(nextResult);
                return aggregate;
            }
            else
            {
                var newAggregate = new AggregateNode();
                newAggregate.Children.Add(aggregate);
                newAggregate.Children.Add(nextResult);
                return newAggregate;
            }
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
