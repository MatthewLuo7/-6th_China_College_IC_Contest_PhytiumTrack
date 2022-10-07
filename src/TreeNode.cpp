/*
 * \file TreeNode.cpp
 * \二叉树结点部分主体
 */

#include "TreeNode.h"

using namespace std;

TreeNode::TreeNode()//无参构造函数
{  
    Ndepth = 0;
    value = 0;
    Parent=NULL; 
    Lchild=NULL;
    Rchild=NULL; 
}

TreeNode::TreeNode(TreeNode* pa, int dep, unsigned char data)//有参构造函数
{  
    value=data; 
    Ndepth=dep;
    Parent=pa;
    Lchild=NULL;
    Rchild=NULL; 
}

void TreeNode::release()//删除当前结点的左右子树
{
    if(Lchild!=NULL)
    {
        Lchild->release();
        delete Lchild;
        Lchild=NULL;
    }
    if(Rchild!=NULL)
    {
        Rchild->release();
        delete Rchild;
        Rchild=NULL;
    }
}