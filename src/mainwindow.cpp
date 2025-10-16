#include "mainwindow.h"
#include "./ui_mainwindow.h"
using namespace std;

/*构造、析构与初始化函数*/
/*****************************************************************************************/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    palette_init();
    setMessageBoxStyle();
    timerPre.setInterval(300);//设置动画间隔为0.3s
    timerIn.setInterval(300);
    timerPost.setInterval(300);
    timerThr.setInterval(300);
    //按钮信号槽连接
    connect(ui->GenButton, &QPushButton::clicked,this, &MainWindow::set_DrawBTree_Mode);

    connectButtonWithCheck(ui->preTraverse, &MainWindow::set_Preorder_Mode);
    connectButtonWithCheck(ui->inTraverse, &MainWindow::set_Inorder_Mode);
    connectButtonWithCheck(ui->postTraverse, &MainWindow::set_Postorder_Mode);

    connectButtonWithCheck(ui->preThreaded, &MainWindow::set_PreThreading_Mode);
    connectButtonWithCheck(ui->inThreaded, &MainWindow::set_InThreading_Mode);
    connectButtonWithCheck(ui->postThreaded, &MainWindow::set_PostThreading_Mode);

    connect(ui->unableThreaded,&QPushButton::clicked,this, &MainWindow::turnoff_Threading);

    connectButtonWithCheck(ui->ThrTraverse, &MainWindow::set_ThreadTraverse_Mode,true);
    //定时器信号槽连接
    connect(&timerPre,&QTimer::timeout,this,&MainWindow::DrawPreorderTraversal);
    connect(&timerIn,&QTimer::timeout,this,&MainWindow::DrawInorderTraversal);
    connect(&timerPost,&QTimer::timeout,this,&MainWindow::DrawPostorderTraversal);
    connect(&timerThr,&QTimer::timeout,this,&MainWindow::DrawThreadedTraversal);

    // 初始化悬浮标签
    m_customTooltip = new QLabel(this);
    m_customTooltip->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    m_customTooltip->setStyleSheet(
        "background: #333; color: white; padding: 5px; border-radius: 4px;"
        );
    m_customTooltip->hide(); // 初始隐藏

    this->setMouseTracking(true);  //启用鼠标跟踪,保证鼠标悬停时显示节点信息
}

MainWindow::~MainWindow()
{
    delete ui;
}
//用于检测错误的用户操作并传递连接信号到槽函数
void MainWindow::connectButtonWithCheck(QPushButton* button,  void (MainWindow::*slotFunc)() , bool checkThreaded) {
    connect(button, &QPushButton::clicked, this, [=]() {
        if (!showTree) {
            QMessageBox::information(this, "错误", "未生成树");
            return;
        }
        if(checkThreaded&&!showThreaded){
            QMessageBox::information(this, "错误", "请先线索化");
            return;
        }
        (this->*slotFunc)();
    });
}

void MainWindow::palette_init()
{
    QPalette palette;
    /*设置窗口背景色*/
    palette.setColor(QPalette::Window, QColor(BABYBLUE));
    this->setPalette(palette);
    /*设置LineEdit的背景色*/
    palette.setColor(QPalette::Base, Qt::white);
    ui->SequenceEdit->setPalette(palette);

    this->setStyleSheet(R"(
        /*设置按钮样式*/
        QPushButton{
            color: #e0e0e0;
            background-color:#234091;
            border-radius: 6px;
            padding: 8px 12px;
            margin: 2px;
            font-size: 16px;
            font-family: 微软雅黑 ;
            text-align: centre;
            border: 2px solid #234091;
            }
        /*设置按钮悬停样式*/
        QPushButton:hover {
            background-color: #000066;
            color: white;
            border: 2px solid #4682B4;
            }
        /*设置全局文本颜色*/
        QLineEdit, QTextEdit {
        color: black;}
        QStatusBar{color:red}
    )");

    /*设置提示颜色背景色*/
    palette.setColor(QPalette::WindowText, Qt::darkBlue);
    ui->prompt_Input->setPalette(palette);
    ui->prompt_PreNPost->setPalette(palette);
    palette.setColor(QPalette::WindowText, Qt::black);
    ui->PromptWords->setPalette(palette);

}

void MainWindow::setMessageBoxStyle() {
    QString style =
        "QMessageBox {"
        "    background-color: #f0f0f0;"  // 背景色
        "    font: 12pt '微软雅黑';"       // 字体
        "}"
        "QMessageBox QLabel {"
        "    color: #333333;"            // 文字颜色
        "}"
        "QMessageBox QPushButton {"
        "    background-color: #4CAF50;" // 按钮背景色
        "    color: white;"              // 按钮文字颜色
        "    padding: 5px 10px;"
        "    border: none;"
        "    border-radius: 4px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "    background-color: #45a049;" // 悬停效果
        "}";
    qApp->setStyleSheet(style);  // 全局生效（所有 QMessageBox）
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    //检查鼠标是否在任意节点区域内
    Node *hoveredNode = nullptr;
    for (auto it = m_nodeRegions.constBegin(); it != m_nodeRegions.constEnd(); ++it) {
        const QRect &rect = it.key();
        Node *node = it.value();
        if (rect.contains(event->pos())) {
            hoveredNode = node;
            break;
        }
    }

    if (hoveredNode) {
        QString info = QString("节点数据: %1\n"
                               "左子节点: %2\n"
                               "右子节点: %3\n"
                               "前驱: %4\n"
                               "后继: %5\n")
                           .arg(hoveredNode->data)
                           .arg(hoveredNode->lchild ? QString(hoveredNode->lchild->data) : "空")
                           .arg(hoveredNode->rchild ? QString(hoveredNode->rchild->data) : "空")
                           .arg(hoveredNode->Pre &&showThreaded? QString(hoveredNode->Pre->data) : "空")
                           .arg(hoveredNode->Post &&showThreaded ? QString(hoveredNode->Post->data) : "空");

        m_customTooltip->setText(info);
        m_customTooltip->move(event->globalPos()+QPoint(5,5));//使用全局坐标
        m_customTooltip->show();
    } else {
        m_customTooltip->hide();
    }
}
//设置禁用/启用按钮，主要用于动画时防止用户进行操作导致遍历失败
void MainWindow::setButtonsOn(const bool& enable)
{
    //给用户一点提示信息：
    if(enable==false)
        statusBar()->showMessage("正在执行遍历动画，请稍候...", 3000); // 3秒后自动消失
    ui->GenButton->setEnabled(enable);
    ui->preTraverse->setEnabled(enable);
    ui->inTraverse->setEnabled(enable);
    ui->postTraverse->setEnabled(enable);
    ui->ThrTraverse->setEnabled(enable);
    ui->preThreaded->setEnabled(enable);
    ui->inThreaded->setEnabled(enable);
    ui->postThreaded->setEnabled(enable);
    ui->unableThreaded->setEnabled(enable);
};
/*****************************************************************************************/

/*槽函数*/
void MainWindow::set_DrawBTree_Mode()
{
    if(ui->SequenceEdit->text()=="")//输入序列为空则不生成树
        return;
    /*初始化二叉树*/
    ptrTree = std::make_unique<BTree>(ui->SequenceEdit->text().toStdString());
    //ptrTree = std::make_unique<BTree>(preseq);
    BTree* T = ptrTree.get();

    /*初始化绘制格式*/
    const int initOffsetX=150,initOffsetY=180;
    //动态计算间距：
    int baseHSpacing = (width()-initOffsetX) / 2;  // 初始水平间距
    int treeHeight = T->getTreeHeight();
    int vSpacing = (height()-initOffsetY) / (treeHeight + 1); // 垂直间距
    //递归计算所有节点位置和叶子节点个数：
    T->calculateNodePositions(T->get_root(), 0, baseHSpacing, vSpacing, initOffsetX, initOffsetY/2);
    showLeavesNum = ptrTree->countLeaves();
    //设置底部信息栏位置：
    this->BottomInformationPos.setX(initOffsetX+50);
    this->BottomInformationPos.setY(height()-initOffsetY/3);

    /*清空节点区域旧数据*/
    m_nodeRegions.clear();
    /*恢复状态位*/
    showTree = true;
    showTraversal = false;
    showThreaded = false;

}

void MainWindow::set_Preorder_Mode()
{
    showTraversal=true;
    timerPre.start();
    setButtonsOn(false);//关闭按钮的响应
    curNode = ptrTree->get_root();//先序遍历起点为根节点
    TraversalSeq.clear();//先清空，防止上一次遍历序列生成结果影响下一次
    ptrTree->clear_visited();
}

void MainWindow::set_Inorder_Mode()
{
    showTraversal=true;
    timerIn.start();
    setButtonsOn(false);//关闭按钮的响应
    curNode = ptrTree->get_root();
    while(curNode->lchild) curNode=curNode->lchild.get();//起始节点为最左侧节点
    TraversalSeq.clear();//先清空，防止上一次遍历序列生成结果影响下一次
    ptrTree->clear_visited();
}

void MainWindow::set_Postorder_Mode()
{
    showTraversal=true;
    timerPost.start();
    setButtonsOn(false);//关闭按钮的响应
    curNode = ptrTree->get_root();
    while(curNode->lchild) curNode=curNode->lchild.get();
    while(curNode->rchild) curNode=curNode->rchild.get();//起始节点为叶节点

    TraversalSeq.clear();//先清空，防止上一次遍历序列生成结果影响下一次
    ptrTree->clear_visited();
}

void MainWindow::turnoff_Threading()
{
    showThreaded=false;
}

void MainWindow::set_PreThreading_Mode()
{
    showThreaded=true;
    ptrTree->PreThreading();
}
void MainWindow::set_InThreading_Mode()
{
    showThreaded=true;
    ptrTree->InThreading();
}
void MainWindow::set_PostThreading_Mode()
{
    showThreaded=true;
    ptrTree->PostThreading();
}
void MainWindow::set_ThreadTraverse_Mode()
{
    showTraversal=true;
    timerThr.start();
    setButtonsOn(false);//关闭按钮的响应
    curNode = ptrTree->get_root();
    while(curNode->Pre) curNode=curNode->Pre;//无前驱的节点做第一个节点

    TraversalSeq.clear();//先清空，防止上一次遍历序列生成结果影响下一次
    ptrTree->clear_visited();
}

void MainWindow::DrawPreorderTraversal()
{
    TraversalSeq.append(curNode->data);//将当前节点信息加入遍历序列
    ptrTree->Preorder_Traversal_FSM(&curNode);
    //打印底部信息栏
    showBottomInfo = true;
    if(!curNode){
        timerPre.stop();
        setButtonsOn(true);//开启按钮的响应
        showTraversal=false;
        return;
    }
    update();
}

void MainWindow::DrawInorderTraversal()
{
    TraversalSeq.append(curNode->data);//将当前节点信息加入遍历序列
    ptrTree->Inorder_Traversal_FSM(&curNode);
    //打印底部信息栏
    showBottomInfo = true;
    if(!curNode){
        timerIn.stop();
        setButtonsOn(true);//开启按钮的响应
        showTraversal=false;
        return;
    }
    update();
}

void MainWindow::DrawPostorderTraversal()
{
    TraversalSeq.append(curNode->data);//将当前节点信息加入遍历序列
    ptrTree->Postorder_Traversal_FSM(&curNode);
    //打印底部信息栏
    showBottomInfo = true;
    if(!curNode){
        timerPost.stop();
        setButtonsOn(true);//开启按钮的响应
        showTraversal=false;
        return;
    }
    update();
}

void MainWindow::DrawThreadedTraversal()
{
    TraversalSeq.append(curNode->data);//将当前节点信息加入遍历序列
    ptrTree->Thread_Traversal_FSM(&curNode);
    //打印底部信息栏
    showBottomInfo = true;
    if(!curNode){
        timerThr.stop();
        setButtonsOn(true);//开启按钮的响应
        showTraversal=false;
        return;
    }
    update();
}
/*****************************************************************************************/

/*绘画事件*/
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);//先暂时不使用event,避免warnings

    QPainter painter(this);
    //设置基础笔刷颜色、粗细
    QPen pen;
    pen.setBrush(Qt::darkGreen);
    pen.setWidth(2);
    painter.setPen(pen);
    //设置基础图形颜色
    painter.setBrush(QColor(152, 251, 152));
    //设置字体与字号
    QFont font;
    font.setFamily("微软雅黑");
    font.setPointSize(10);
    painter.setFont(font);
    //设置抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);

    //绘制二叉树；
    if(showTree)
        DrawTree(painter, ptrTree->get_root());

    if(showThreaded){
        DrawThreadingLine(painter);
        DrawTree(painter, ptrTree->get_root());
    }

    if(showBottomInfo){
        painter.save();
        font.setPointSize(14);//调大字号
        painter.setFont(font);
        painter.drawText(BottomInformationPos,QString("遍历序列 ：")+TraversalSeq);
        painter.drawText(BottomInformationPos+QPoint(0,50),QString("叶子节点个数 ：")+ QString::number(showLeavesNum));
        painter.restore();
    }

    if(showTraversal)
        DrawTraverse(painter);
    update();
}

//递归方式实现树的绘制
void MainWindow::DrawTree(QPainter& painter, Node* node)
{
    if (!node) return;

    // 绘制左子树连线
    if (node->lchild) {
        painter.drawLine(node->pos, node->lchild->pos);
        DrawTree(painter, node->lchild.get());
    }
    // 绘制右子树连线
    if (node->rchild) {
        painter.drawLine(node->pos, node->rchild->pos);
        DrawTree(painter, node->rchild.get());
    }
    //绘制当前节点
    painter.drawEllipse(node->pos, 15, 15);
    painter.drawText(node->pos-QPoint(5,-5), QString(node->data));
    //计算悬停区域
    // 计算节点绘制区域（示例：30x30的矩形区域）
    QRect nodeRect(node->pos.x() - 15, node->pos.y() - 15, 30, 30);

    // 保存节点区域映射
    m_nodeRegions.insert(nodeRect, node);
}

//高亮标记当前节点，表示遍历到当前节点
void MainWindow::DrawTraverse(QPainter& painter)
{
    if(!curNode)  return;//遍历结束

    painter.setBrush(Qt::yellow);
    painter.drawEllipse(curNode->pos, 15, 15);
    painter.drawText(curNode->pos-QPoint(5,-5), QString(curNode->data));
    curNode->visited = true;//当绘制完高光后再置为已访问
}

//绘制带箭头的曲线
void MainWindow::DrawArrowCurve(QPainter& painter,const QPoint& start,const QPoint& end,const QColor& color)
{
    painter.save();
    QPen pen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    //计算控制点，使曲线控制点在三分点处
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();
    QPointF ctrl1(start.x() , start.y() + dy / 3);
    QPointF ctrl2(start.x() + 2 * dx / 3, end.y());
    if(start.y() == end.y()){
        ctrl1 = QPointF(start.x() +dx*0.3, start.y()+ 15);//垂直方向偏移量为1
        ctrl2 = QPointF(start.x() +dx*0.7, start.y()- 15);
    }
    //创建并绘制贝塞尔曲线路径
    QPainterPath path;
    path.moveTo(start);
    path.cubicTo(ctrl1, ctrl2, end);
    painter.drawPath(path);

    //计算箭头位置和角度
    qreal angle = std::atan2(end.y() - ctrl2.y(), end.x() - ctrl2.x());
    qreal arrowSize = 5;
    QPointF arrowP1 = end - QPointF(arrowSize * std::cos(angle + M_PI / 6),
                                    arrowSize * std::sin(angle + M_PI / 6));
    QPointF arrowP2 = end - QPointF(arrowSize * std::cos(angle - M_PI / 6),
                                    arrowSize * std::sin(angle - M_PI / 6));

    //绘制箭头
    QPolygonF arrowHead;
    arrowHead << end << arrowP1 << arrowP2;//end,arrowP1，arrowP2三点构成等边三角形
    painter.drawPolygon(arrowHead);

    painter.restore();
}


void MainWindow::DrawThreadingLine(QPainter& painter)
{
    Node* cur=ptrTree->get_root();
    if(!cur)return;
    while(cur->Pre&& cur->Pre != cur)//将无前驱的节点作为第一个节点
        cur = cur->Pre;
    while(cur){
        if(!cur->lchild&&cur->Pre)//无左孩子，显示指向前驱的线
            DrawArrowCurve(painter,cur->pos,cur->Pre->pos,Qt::red);
        if(!cur->rchild&&cur->Post)//无右孩子，显示指向后继的线
            DrawArrowCurve(painter,cur->pos,cur->Post->pos,Qt::darkMagenta);
        cur=cur->Post;
    }
    update();
}


