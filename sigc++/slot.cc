#include <sigc++/slot.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif // SIGC_CXX_NAMESPACES

SlotDependent::~SlotDependent()
  {}

SlotDependent::Dep::~Dep()
  {}

SlotNode::SlotNode(void):next_(0)
  {}

SlotNode::~SlotNode(void)
  {}

SlotData::~SlotData(void)
  {}

void SlotDependent::Dep::erase()
  {
   parent->invalid();
  }

void SlotDependent::erase()
  {
   dep.parent->invalid();
  }

void SlotData::connect()
  {
   reference();
   invalid();
   set_weak();
   unreference();
  } 

void SlotList_::clear()
  {
   Iterator current=begin();
   Iterator next=current;
   head_=0;
    
   while (current!=end())
     {
      next++;
      delete current.node();
      current=next;
     }
  }

SlotList_::Iterator SlotList_::insert_direct(Iterator pos,NodeType *node)
  {
   NodeType *loc=pos.node();
   if (pos==begin())
     {
      node->next_=head_;
      head_=node;
     } 
   else
     {
      Iterator i,j;  
      j=begin();
      while (i=j++,j!=end()&&j!=pos);

      NodeType *parent=i.node();
      parent->next_=node;
      node->next_=loc;
     }
   return Iterator(node);
  }

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif // SIGC_CXX_NAMESPACES


