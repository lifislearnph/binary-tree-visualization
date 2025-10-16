#include "BTree_base.h"
#include<stack>
#include <queue>

/*利用先序序列建树，序列形如"ABC#D"
*输入：preorder:先序序列，字符串型，不能为空串
*/
BTree::BTree(const std::string& preorder)
{
    if (preorder.empty() || preorder[0] == '#') {
        return;//空树或非法输入，直接返回
    }
    //建立根节点
    root = std::make_unique<Node>();
    root->data = preorder[0];

    Node* cur=root.get();//指向当前节点的指针
    std::stack<Node*>s;//利用栈进行非递归建树
    s.push(cur);//压入根节点

    for(int i=1;i<preorder.length();i++){
        const char d=preorder[i];

        if(d=='#'){
            if(s.empty()) break;
            cur = s.top();
            s.pop();
            cur->poped = true;
        }
        else{
            auto newNode = std::make_unique<Node>();
            newNode->data = d;
            newNode->parent = cur;

            if(cur->lchild==nullptr&&cur->poped==false){
                cur->lchild = std::move(newNode);
                cur = cur->lchild.get();
            }
            else{
                cur->rchild = std::move(newNode);
                cur = cur->rchild.get();
            }
            s.push(cur);
        }
    }
}

/*计算树中每个节点坐标，方便绘图
depth : 当前节点的深度
hSpacing : 每个节点区域水平宽度（随着每层节点的增多逐层减少）
vSpacing : 每个节点区域高度
xOffset : x方向偏移量
yOffset : y方向偏移量
*/
void BTree::calculateNodePositions(Node* node, const int depth,
                                   const int hSpacing, const int vSpacing, int xOffset,const int yOffset)
{
    if (!node) return;

    //中序递归遍历左子树，为其标注坐标
    if (node->lchild)
        calculateNodePositions(node->lchild.get(), depth + 1, hSpacing / 2, vSpacing, xOffset, yOffset);

    //当前节点位置
    node->pos.setX(xOffset + hSpacing);
    node->pos.setY(yOffset + depth * vSpacing);

    //更新右子树的偏移量
    xOffset += hSpacing;

    // 递归右子树
    if (node->rchild)
        calculateNodePositions(node->rchild.get(), depth + 1, hSpacing / 2, vSpacing, xOffset, yOffset);
}

//非递归方式计算树高
int BTree::getTreeHeight()
{
    if (!root) return 0;
    //利用先序遍历至叶节点获得树高
    std::queue<Node*> q;
    q.push(root.get());
    int height = 0;

    while (!q.empty()) {
        height++;

        for (int i = 0; i < q.size(); ++i) {//遍历当前层所有节点
            Node* cur = q.front();
            q.pop();
            if (cur->lchild) q.push(cur->lchild.get());
            if (cur->rchild) q.push(cur->rchild.get());
        }
    }
    return height;
}

//非递归方式计算叶子节点个数
int BTree::countLeaves()
{
    if (root == nullptr) return 0;

    std::stack<Node*> s;
    s.push(root.get());
    int count = 0;

    while (!s.empty()) {
        Node* node = s.top();
        s.pop();
        if (node->lchild == nullptr && node->rchild == nullptr)
            count++;

        if (node->rchild) s.push(node->rchild.get());
        if (node->lchild) s.push(node->lchild.get());
    }
    return count;
}
//先序遍历方式清空 visited 标记
void  BTree::clear_visited()
{
    std::stack<Node*> s;
    s.push(root.get());
    while (!s.empty()) {
        Node* node = s.top();
        s.pop();
        node->visited=false;
        if (node->rchild) s.push(node->rchild.get());
        if (node->lchild) s.push(node->lchild.get());//左子树后压栈（保证先访问）
    }
}

/*先序遍历状态机：
 * 输入当前节点，输出先序遍历顺序下的下一个节点*/
void BTree::Preorder_Traversal_FSM(Node** curNode)
{
    do{
        if((*curNode)->lchild&&!(*curNode)->lchild->visited){
            (*curNode) = (*curNode)->lchild.get();
        }
        else if((*curNode)->rchild&&!(*curNode)->rchild->visited){
            (*curNode) = (*curNode)->rchild.get();
        }
        else
            (*curNode) = (*curNode)->parent;//没有子节点或子节点都已被访问过，则返回父节点
    }while(*curNode!=nullptr&&(*curNode)->visited);
}

/*中序遍历状态机：
 * 输入当前节点，输出中序遍历顺序下的下一个节点*/
void BTree::Inorder_Traversal_FSM(Node** curNode)
{
    while(1){
        if((*curNode)->lchild&&!(*curNode)->lchild->visited){//左子树不为空且左子树未被访问过
            (*curNode) = (*curNode)->lchild.get();
        }
        else if(!(*curNode)->lchild&&!(*curNode)->visited){//左子树为空且自身没被访问过
            break;
        }
        else{
            if((*curNode)->rchild&&!(*curNode)->rchild->visited){//右子树不为空且未被访问，更新为右子树
                (*curNode) = (*curNode)->rchild.get();
            }
            else if((*curNode)->parent){
                (*curNode) = (*curNode)->parent;
                if(!(*curNode)->visited) break;//如果父节点未被访问过,才能退出
            }
            else{//假如此时parent为空，则说明返回到根节点，设置退出条件
                (*curNode) = (*curNode)->parent;
                break;
            }
        }
    }
}

/* 后序遍历状态机：
 * 输入当前节点，输出后序遍历顺序下的下一个节点*/
void BTree::Postorder_Traversal_FSM(Node** curNode)
{
    while(1){
        if((*curNode)->lchild&&!(*curNode)->lchild->visited){//左子树不为空且左子树未被访问过
            (*curNode) = (*curNode)->lchild.get();
        }
        else if((*curNode)->rchild&&!(*curNode)->rchild->visited){//右子树不为空且未被访问过
            (*curNode) = (*curNode)->rchild.get();
        }
        else if(!(*curNode)->visited)
                break;
        else if((*curNode)->parent)
                (*curNode) = (*curNode)->parent;
        else{//假如此时parent为空，则说明返回到根节点，设置退出条件
            (*curNode) = (*curNode)->parent;
            break;
        }
    }
}

//通过非递归先序遍历方式，使二叉树先序线索化
void BTree::PreThreading()
{
    if (root == nullptr) return;

    std::stack<Node*> s;
    s.push(root.get());

    Node* lastNode = nullptr;
    while (!s.empty()) {
        Node* cur = s.top();
        s.pop();
        //设置前驱和后继
        cur->Pre = lastNode;

        if(lastNode) lastNode->Post = cur;
        lastNode = cur;

        if (cur->rchild) s.push(cur->rchild.get());
        if (cur->lchild) s.push(cur->lchild.get());//左子树后压栈（保证先访问）
    }
    lastNode->Post = nullptr;//设置最后一个后继
}

//通过非递归中序遍历方式，使二叉树中序线索化
void BTree::InThreading()
{
    if (!root) return;

    std::stack<Node*> s;
    Node* cur = root.get(),*lastNode = nullptr;
    while (cur != nullptr || !s.empty()) {

        while(cur){
            s.push(cur);
            cur = cur->lchild.get();
        }
        cur = s.top();
        s.pop();
        //设置前驱和后继
        cur->Pre = lastNode;
        if(lastNode) lastNode->Post = cur;
        lastNode = cur;

        cur =cur->rchild.get();
    }
    lastNode->Post = nullptr;//将最后一个节点的后继设为nullptr,防止循环读取
}

//通过非递归后序遍历方式，使二叉树后序线索化
void BTree::PostThreading()
{
    if (!root) return;

    std::stack<Node*> s;
    Node* cur = root.get(),*lastNode = nullptr;
    while (cur != nullptr || !s.empty()) {
        if (cur) {
            s.push(cur);
            cur = cur->lchild.get();
        } else{
            Node* peekNode = s.top();
            // 如果右子树存在且未被访问过
            if (peekNode->rchild && lastNode != peekNode->rchild.get()) {
                cur = peekNode->rchild.get();
            } else {
                //设置前驱和后继
                peekNode->Pre = lastNode;
                if(lastNode) lastNode->Post = peekNode;

                lastNode = peekNode;
                s.pop();
            }
        }

    }
    lastNode->Post = nullptr;//将最后一个节点的后继设为nullptr,防止循环读取
}

void BTree::Thread_Traversal_FSM(Node** curNode)
{
    (*curNode) = (*curNode)->Post;
}


