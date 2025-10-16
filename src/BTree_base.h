#ifndef BTREE_BASE_H
#define BTREE_BASE_H

#include<string>
#include<memory>
#include<QPoint>
#include<QToolTip>
#include<QLabel>

struct Node;
class BTree
{
public:
    BTree(const std::string& preorder);//利用先序序列建树，序列形如"ABC#D"
    ~BTree()=default;
    void calculateNodePositions(Node* node, const int depth, const int hSpacing, const int vSpacing, int xOffset,const int yOffset);
    int getTreeHeight();//计算树高，用于安排节点在y方向的间距
    int countLeaves();//非递归方式计算叶子节点个数
    void clear_visited();//每次绘制遍历动画前清空访问标记

    inline Node* get_root(){return root.get();};

    void Preorder_Traversal_FSM(Node** curNode);//先序遍历状态转移函数
    void Inorder_Traversal_FSM(Node** curNode);//中序遍历状态转移函数
    void Postorder_Traversal_FSM(Node** curNode);//后序遍历状态转移函数

    void PreThreading();
    void InThreading();
    void PostThreading();
    void Thread_Traversal_FSM(Node** curNode);
private:
    std::unique_ptr<Node> root = nullptr;
};

class Node{
public:
    char data ='@';

    std::unique_ptr<Node> lchild =nullptr;//使用智能指针管理内存
    std::unique_ptr<Node> rchild =nullptr;
    Node* parent =nullptr;
    bool poped =false;//被栈弹出，方便检测是否已经入过栈
    bool visited =false;//当前节点被遍历标记
    //线索化(由于使用智能指针，所以直接设置前置指针和后置指针，而不是设置标签)
    Node* Pre = nullptr;//指向前驱
    Node* Post = nullptr;//指向后继

    QPoint pos ={0,0};//记录节点的坐标

    QGraphicsProxyWidget* m_tooltipProxy = nullptr;

    void showToolTip();
    void hideToolTip();
};
#endif // BTREE_BASE_H


