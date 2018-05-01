using System.Collections.Generic;

namespace Lasagne.Compiler.Ast
{
    public class Implementation : Node
    {
        public string Name { get; set; }
        public List<Function> Methods { get; } = new List<Function>();

        public void AddMethods(IEnumerable<Function> methods)
        {
            Methods.AddRange(methods);
        }
    }
}
