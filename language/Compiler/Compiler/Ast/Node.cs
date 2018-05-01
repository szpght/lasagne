using System.Collections.Generic;

namespace Lasagne.Compiler.Ast
{
    public abstract class Node
    {
        public virtual List<Node> Children { get; } = new List<Node>();

        public virtual int StartLine { get; set; }
        public virtual int StartPosition { get; set; }
        public virtual int StopLine { get; set; }
        public virtual int StopPosition { get; set; }

        public void AddChildren(IEnumerable<Node> nodes)
        {
            Children.AddRange(nodes);
        }
    }
}
