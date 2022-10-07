/*
 * \file Tree.cpp
 * \二叉树部分主体
 */

#include "Tree.h"

using namespace std;

Tree::Tree()//构造函数
{
    root=new TreeNode;
    curp=root;
}

Tree::~Tree()//析构函数 
{
    if(root!=NULL)
    {
        root->release();
        delete root;
        root=NULL;
    }
}

TreeNode* Tree::getroot()//获得根结点 
{
    return root;
}

void Tree::AddNode(unsigned short code_value, int code_length)
{
    // printf("current Ndepth: %d\n",curp->Ndepth);
    // printf("current value: %d\n",curp->value);
    if(code_length <= curp->Ndepth)
    {
        printf("Creating Tree Error!!!\n");
        return;
    }
    else
    {
        if(curp->Lchild == NULL)
        {
            if((code_length-1) == curp->Ndepth)
            {
                TreeNode* p = new TreeNode(curp,code_length,code_value);
                curp->Lchild = p;
            }
            else
            {
                TreeNode* p = new TreeNode(curp,(curp->Ndepth+1),0);
                curp->Lchild = p;
                curp = curp->Lchild;
                AddNode(code_value,code_length);
            }
        }
        else if(curp->Rchild == NULL)
        {
            if((code_length-1) == curp->Ndepth)
            {
                TreeNode* p = new TreeNode(curp,code_length,code_value);
                curp->Rchild = p;
            }
            else
            {
                TreeNode* p = new TreeNode(curp,curp->Ndepth + 1,0);
                curp->Rchild = p;
                curp = curp->Rchild;
                AddNode(code_value,code_length);
            }
        }
        else
        {
            curp = curp->Parent;
            AddNode(code_value,code_length);
        }
    }
}

void Tree::PreOrder(TreeNode * b){ //先序遍历
       if(b != NULL){
           printf("%d\n",b->value); //访问根节点
           PreOrder(b->Lchild);
           PreOrder(b->Rchild);
       }
   }