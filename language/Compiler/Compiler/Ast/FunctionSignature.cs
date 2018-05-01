using System.Collections.Generic;

namespace Lasagne.Compiler.Ast
{
    public class FunctionSignature : Node
    {
        public string Name { get; set; }
        public List<TypedParameter> Parameters { get; } = new List<TypedParameter>();
        public TypeNode ReturnedType { get; set; }

        public void AddParameters(IEnumerable<TypedParameter> parameters)
        {
            Parameters.AddRange(parameters);
        }
    }
}
