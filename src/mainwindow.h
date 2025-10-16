#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QPainter>
#include<QPushButton>
#include<QTimer>
#include<QPainterPath>
#include<QMessageBox>
#include<QMouseEvent>
#include"BTree_base.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/*定义主窗口*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void connectButtonWithCheck(QPushButton* button, void (MainWindow::*slotFunc)(), bool checkThreaded=false);
private slots:
    void set_DrawBTree_Mode();//实现DrawBTree的初始化
    void set_Preorder_Mode();//响应先序遍历信号
    void set_Inorder_Mode();//响应中序遍历信号
    void set_Postorder_Mode();//响应后序遍历信号
    void set_ThreadTraverse_Mode();

    void turnoff_Threading();
    void set_PreThreading_Mode();//响应先序线索化信号
    void set_InThreading_Mode();//响应中序线索化信号
    void set_PostThreading_Mode();//响应后序线索化信号

    void DrawPreorderTraversal();//响应定时器，绘制先序遍历过程
    void DrawInorderTraversal();//响应定时器，绘制中序遍历过程
    void DrawPostorderTraversal();//响应定时器，绘制后序遍历过程
    void DrawThreadedTraversal();//响应定时器，绘制线索遍历过程
private:
    Ui::MainWindow *ui;
    bool showTree = false;
    bool showThreaded = false;    // 是否显示线索化
    bool showTraversal = false;  // 是否显示遍历动画
    bool showBottomInfo = false;//是否显示底部信息栏

    std::unique_ptr<BTree> ptrTree;  // 当前树的指针
    int showLeavesNum;//显示叶子节点个数
    QPoint BottomInformationPos;//底部信息栏位置
    QString TraversalSeq;//记录遍历序列

//用于实现动画的成员：
    QTimer timerPre,timerIn,timerPost,timerThr;
    Node* curNode = nullptr;//当前遍历到的节点
//鼠标悬停显示信息的区域
    QHash<QRect, Node*> m_nodeRegions; // 存储节点区域与节点指针的映射
    QLabel *m_customTooltip;    //自定义tooltip，用于打印节点信息
    void mouseMoveEvent(QMouseEvent *event);
//成员函数：
    void setButtonsOn(const bool& enable);//设置禁用/启用按钮
    void palette_init();//初始化背景、图标和图形的样式
    void setMessageBoxStyle();//初始化QMessageBox样式
    //画图事件函数：
    void paintEvent(QPaintEvent *event);
    void DrawTree(QPainter& painter, Node* node);
    void DrawTraverse(QPainter& painter);//遍历动画，用高亮表示
    void DrawArrowCurve(QPainter& painter,const QPoint& start,const QPoint& end,const QColor& color);//绘制线索化的箭头
    void DrawThreadingLine(QPainter& painter);//画出线索化过后的痕迹
};

/*声明需要使用的颜色*/
const QColor BABYBLUE{191, 239, 255};
const QColor STEELBLUE{70, 130, 180};

#endif // MAINWINDOW_H
