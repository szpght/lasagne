using System;
using System.Text;
using Antlr4.Runtime;

namespace Compiler
{
    public class LasagneCompiler
    {
        public void Compile()
        {
            Console.WriteLine("Compiling");
            var text = "def hello () { question = 1 answer = 42 }";
            var input = new AntlrInputStream(text);
            var lexer = new LasagneLexer(input);
            var commonTokenStream = new CommonTokenStream(lexer);
            var parser = new LasagneParser(commonTokenStream);
            var visitor = new LasagneBaseVisitor<string>();
            var tree = parser.program();
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
