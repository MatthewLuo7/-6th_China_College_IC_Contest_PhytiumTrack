/*
 * \file Tree.h
 * \二叉树部分头文件
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "TreeNode.h"

class Tree
{
private:
    TreeNode* root;//根结点
    TreeNode* curp;//当前位置
public:
        Tree();//构造函数 
        ~Tree();//析构函数
        void AddNode(unsigned short code_value, int code_length);
        void PreOrder(TreeNode * b);
        TreeNode* getroot();//获得根结点
        //unsigned char Huffdecode(); 
};