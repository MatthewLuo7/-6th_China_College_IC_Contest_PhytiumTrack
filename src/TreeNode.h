/*
 * \file TreeNode.h
 * \二叉树结点部分头文件
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>

class TreeNode//二叉树结点类 
{   
	public:
	    TreeNode();//无参构造函数
		TreeNode(TreeNode* pa, int dep, unsigned char data);//已知数据域的构造函数
		void release();//删除当前结点的左右子树

        int Ndepth;		
		unsigned char value;//数据域
		TreeNode* Parent; 
		TreeNode* Lchild;//左孩子指针
		TreeNode* Rchild;//右孩子指针 
};