using System.Collections.Generic;
using System.Linq;

namespace Lasagne.Compiler.Ast
{
    public class Node
    {
        public virtual List<Node> Children { get; } = new List<Node>();

        public virtual Node Child // ha ha hack
        {
            get => Children.FirstOrDefault();
            set => Children.Insert(0, value);
        }

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
