#include "GraphicsView.hpp"

#include <QtWidgets/QGraphicsScene>

#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtWidgets/QMenu>

#include <QtCore/QRectF>
#include <QtCore/QPointF>
#include <QtCore/QDebug>

#include <QtOpenGL>
#include <QtWidgets>

#include <iostream>
#include <cmath>

#include "ConnectionGraphicsObject.hpp"
#include "NodeGraphicsObject.hpp"
#include "BasicGraphicsScene.hpp"
#include "StyleCollection.hpp"

using QtNodes::GraphicsView;
using QtNodes::BasicGraphicsScene;

GraphicsView::
    GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
      , _clearSelectionAction(Q_NULLPTR)
      , _deleteSelectionAction(Q_NULLPTR)
      , _copySelectionAction(Q_NULLPTR)
      , _cutSelectionAction(Q_NULLPTR)
      , _pasteClipboardAction(Q_NULLPTR)
      , _createGroupFromSelectionAction(Q_NULLPTR)
      , _loadGroupAction(Q_NULLPTR)
{
  setDragMode(QGraphicsView::ScrollHandDrag);
  setRenderHint(QPainter::Antialiasing);

  auto const &flowViewStyle = StyleCollection::flowViewStyle();

  setBackgroundBrush(flowViewStyle.BackgroundColor);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

  setCacheMode(QGraphicsView::CacheBackground);
  setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

          //setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
}


GraphicsView::
    GraphicsView(BasicGraphicsScene *scene, QWidget *parent)
    : GraphicsView(parent)
{
  setScene(scene);
}


QAction*
    GraphicsView::
    clearSelectionAction() const
{
  return _clearSelectionAction;
}


QAction*
    GraphicsView::
    deleteSelectionAction() const
{
  return _deleteSelectionAction;
}

QAction*
    GraphicsView::
    copySelectionAction() const
{
  return _copySelectionAction;
}

QAction*
    GraphicsView::
    cutSelectionAction() const
{
  return _cutSelectionAction;
}

QAction*
    GraphicsView::
    pasteClipboardAction() const
{
  return _pasteClipboardAction;
}

QAction*
    GraphicsView::
    createGroupFromSelectionAction() const
{
  return _createGroupFromSelectionAction;
}

QAction*
    GraphicsView::
    loadGroupAction() const
{
  return _loadGroupAction;
}

void
    GraphicsView::
    setScene(BasicGraphicsScene *scene)
{
  QGraphicsView::setScene(scene);

          // setup actions
  delete _clearSelectionAction;
  _clearSelectionAction = new QAction(QStringLiteral("Clear Selection"), this);
  _clearSelectionAction->setShortcut(Qt::Key_Escape);
  connect(_clearSelectionAction, &QAction::triggered,
          scene, &QGraphicsScene::clearSelection);
  addAction(_clearSelectionAction);

  delete _deleteSelectionAction;
  _deleteSelectionAction = new QAction(QStringLiteral("Delete Selection"), this);
  _deleteSelectionAction->setShortcut(Qt::Key_Delete);
  connect(_deleteSelectionAction, &QAction::triggered,
          this, &GraphicsView::deleteSelectedObjects);
  addAction(_deleteSelectionAction);

  if (_copySelectionAction != nullptr)
    delete _copySelectionAction;
  _copySelectionAction = new QAction(QStringLiteral("Copy"), this);
  _copySelectionAction->setShortcut(QKeySequence::Copy);
  _copySelectionAction->setEnabled(false);
  connect(_copySelectionAction, &QAction::triggered, this, &GraphicsView::copySelectionToClipboard);
  addAction(_copySelectionAction);

  if (_cutSelectionAction != nullptr)
    delete _cutSelectionAction;
  _cutSelectionAction = new QAction(QStringLiteral("Cut"), this);
  _cutSelectionAction->setShortcut(QKeySequence::Cut);
  _cutSelectionAction->setEnabled(false);
  connect(_cutSelectionAction, &QAction::triggered, this, &GraphicsView::cutSelectionToClipboard);
  addAction(_cutSelectionAction);

  if (_pasteClipboardAction != nullptr)
    delete _pasteClipboardAction;
  _pasteClipboardAction = new QAction(QStringLiteral("Paste"), this);
  _pasteClipboardAction->setShortcut(QKeySequence::Paste);
  _pasteClipboardAction->setEnabled(false);
  connect(_pasteClipboardAction, &QAction::triggered, this, &GraphicsView::pasteFromClipboard);
  addAction(_pasteClipboardAction);

  if (_createGroupFromSelectionAction != nullptr)
    delete _createGroupFromSelectionAction;
  _createGroupFromSelectionAction = new QAction(
      QStringLiteral("Create group from selection"), this);
  _createGroupFromSelectionAction->setEnabled(false);
  connect(_createGroupFromSelectionAction, &QAction::triggered,
          [&, this]()
          {
//            scene->createGroupFromSelection();
          });
  addAction(_createGroupFromSelectionAction);

  if (_loadGroupAction != nullptr)
    delete _loadGroupAction;
  _loadGroupAction = new QAction(QStringLiteral("Load Group..."), this);
  _createGroupFromSelectionAction->setEnabled(true);
  connect(_loadGroupAction, &QAction::triggered, this, &GraphicsView::handleLoadGroup);
  addAction(_loadGroupAction);
}

void
    GraphicsView::
    centerScene()
{
  if (scene())
  {
    scene()->setSceneRect(QRectF());

    QRectF sceneRect = scene()->sceneRect();

    if (sceneRect.width() > this->rect().width() ||
        sceneRect.height() > this->rect().height())
    {
      fitInView(sceneRect, Qt::KeepAspectRatio);
    }

    centerOn(sceneRect.center());
  }
}


void
    GraphicsView::
    contextMenuEvent(QContextMenuEvent *event)
{
  if (itemAt(event->pos()))
  {
    QGraphicsView::contextMenuEvent(event);
    return;
  }

  auto const scenePos = mapToScene(event->pos());

  QMenu * menu = nodeScene()->createSceneMenu(scenePos);

  if (menu)
  {
    menu->exec(event->globalPos());
  }
}


void
    GraphicsView::
    wheelEvent(QWheelEvent *event)
{
  QPoint delta = event->angleDelta();

  if (delta.y() == 0)
  {
    event->ignore();
    return;
  }

  double const d = delta.y() / std::abs(delta.y());

  if (d > 0.0)
    scaleUp();
  else
    scaleDown();
}


void
    GraphicsView::
    scaleUp()
{
  double const step   = 1.2;
  double const factor = std::pow(step, 1.0);

  QTransform t = transform();

  if (t.m11() > 2.0)
    return;

  scale(factor, factor);
}


void
    GraphicsView::
    scaleDown()
{
  double const step   = 1.2;
  double const factor = std::pow(step, -1.0);

  scale(factor, factor);
}


void
    GraphicsView::
    deleteSelectedObjects()
{
  // Delete the selected connections first, ensuring that they won't be
  // automatically deleted when selected nodes are deleted (deleting a
  // node deletes some connections as well)
  for (QGraphicsItem * item : scene()->selectedItems())
  {
    if (auto c = qgraphicsitem_cast<ConnectionGraphicsObject*>(item))
    {
      nodeScene()->graphModel().deleteConnection(c->connectionId());
    }
  }

          // Delete the nodes; this will delete many of the connections.
          // Selected connections were already deleted prior to this loop,
          // otherwise qgraphicsitem_cast<NodeGraphicsObject*>(item) could be a
          // use-after-free when a selected connection is deleted by deleting
          // the node.
  for (QGraphicsItem * item : scene()->selectedItems())
  {
    if (auto n = qgraphicsitem_cast<NodeGraphicsObject*>(item))
    {
      nodeScene()->graphModel().deleteNode(n->nodeId());
    }
  }
}


void
    GraphicsView::
    keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
  {
    case Qt::Key_Shift:
      setDragMode(QGraphicsView::RubberBandDrag);
      break;

    default:
      break;
  }

  QGraphicsView::keyPressEvent(event);
}


void
    GraphicsView::
    keyReleaseEvent(QKeyEvent *event)
{
  switch (event->key())
  {
    case Qt::Key_Shift:
      setDragMode(QGraphicsView::ScrollHandDrag);
      break;

    default:
      break;
  }
  QGraphicsView::keyReleaseEvent(event);
}


void
    GraphicsView::
    mousePressEvent(QMouseEvent *event)
{
  QGraphicsView::mousePressEvent(event);
  if (event->button() == Qt::LeftButton)
  {
    _clickPos = mapToScene(event->pos());
  }
}


void
    GraphicsView::
    mouseMoveEvent(QMouseEvent *event)
{
  QGraphicsView::mouseMoveEvent(event);
  if (scene()->mouseGrabberItem() == nullptr && event->buttons() == Qt::LeftButton)
  {
    // Make sure shift is not being pressed
    if ((event->modifiers() & Qt::ShiftModifier) == 0)
    {
      QPointF difference = _clickPos - mapToScene(event->pos());
      setSceneRect(sceneRect().translated(difference.x(), difference.y()));
    }
  }
}


void
    GraphicsView::
    drawBackground(QPainter* painter, const QRectF &r)
{
  QGraphicsView::drawBackground(painter, r);

  auto drawGrid =
      [&](double gridStep)
  {
    QRect windowRect = rect();
    QPointF tl       = mapToScene(windowRect.topLeft());
    QPointF br       = mapToScene(windowRect.bottomRight());

    double left   = std::floor(tl.x() / gridStep - 0.5);
    double right  = std::floor(br.x() / gridStep + 1.0);
    double bottom = std::floor(tl.y() / gridStep - 0.5);
    double top    = std::floor(br.y() / gridStep + 1.0);

            // vertical lines
    for (int xi = int(left); xi <= int(right); ++xi)
    {
      QLineF line(xi * gridStep, bottom * gridStep,
                  xi * gridStep, top * gridStep);

      painter->drawLine(line);
    }

            // horizontal lines
    for (int yi = int(bottom); yi <= int(top); ++yi)
    {
      QLineF line(left * gridStep, yi * gridStep,
                  right * gridStep, yi * gridStep);
      painter->drawLine(line);
    }
  };

  auto const &flowViewStyle = StyleCollection::flowViewStyle();

  QPen pfine(flowViewStyle.FineGridColor, 1.0);

  painter->setPen(pfine);
  drawGrid(15);

  QPen p(flowViewStyle.CoarseGridColor, 1.0);

  painter->setPen(p);
  drawGrid(150);
}


void
    GraphicsView::
    showEvent(QShowEvent *event)
{
  QGraphicsView::showEvent(event);

  scene()->setSceneRect(this->rect());
  centerScene();
}


BasicGraphicsScene *
    GraphicsView::
    nodeScene()
{
  return dynamic_cast<BasicGraphicsScene*>(scene());
}
