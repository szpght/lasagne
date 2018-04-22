using Compiler;

namespace CompilerRunner
{
    class Program
    {
        static void Main(string[] args)
        {
            var compiler = new LasagneCompiler();
            compiler.Compile();
        }
    }
}
