using System;
using System.Text;
using Antlr4.Runtime;
using Lasagne.Compiler.Ast;

namespace Lasagne.Compiler
{
    public class LasagneCompiler
    {
        public void Compile()
        {
            Console.WriteLine("Compiling");
            var text = "enum FullEnum {value1 value2: 42} enum EmptyEnum {}";
            text += " def hello (arg1: type1, arg2: type2): type3 { question = 1 answer = 42 }";
            text += "struct EmptyStruct {} struct StructFullOfGood {var1: type1 var2: type2}";
            text += "impl StructFullOfGood { def pub hello () {} def helloImpl(){ question = 1 answer = 42 }}";
            text += "def pub variableManipulations() { let constant = 1 var variable = 2 variable = constant }";
            var input = new AntlrInputStream(text);
            var lexer = new LasagneLexer(input);
            var commonTokenStream = new CommonTokenStream(lexer);
            var parser = new LasagneParser(commonTokenStream);
            var tree = parser.program();
            var nodeBuilder = new NodeBuilder();
            var visitor = new Visitor(nodeBuilder);
            var program = visitor.VisitProgram(tree);
            var ctx = tree.ToStringTree(parser);
            Console.WriteLine(FormatSexp(ctx));
         }

        public string FormatSexp(string sexp)
        {
            var indent = 0;
            const int step = 2;
            var output = new StringBuilder();
            for (int i = 0; i < sexp.Length; ++i)
            {
                var ch = sexp[i];
                if (ch == '(')
                {
                    output.Append("\n");
                    for (int j = 0; j < indent; ++j)
                    {
                        output.Append(' ');
                    }

                    output.Append('(');
                    indent += step;
                }
                else if (ch == ')')
                {
                    indent -= step;
                    output.Append(')');
                }
                else
                {
                    output.Append(ch);
                }
            }

            return output.ToString();
        }
    }
}
