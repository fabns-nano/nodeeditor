#include "GroupGraphicsObject.hpp"

#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

#include "NodeGraphicsObject.hpp"
#include "qpainter.h"

using QtNodes::GroupGraphicsObject;
using QtNodes::NodeGraphicsObject;
using QtNodes::NodeGroup;

IconGraphicsItem::IconGraphicsItem(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
{}

IconGraphicsItem::IconGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsPixmapItem(pixmap, parent)
{
    _scaleFactor = _iconSize / pixmap.size().width();
    setScale(_scaleFactor);
}

double IconGraphicsItem::scaleFactor() const
{
    return _scaleFactor;
}

GroupGraphicsObject::GroupGraphicsObject(DataFlowGraphicsScene &scene, NodeGroup &nodeGroup)
    : _scene(scene)
    , _group(nodeGroup)
    , _possibleChild(InvalidNodeId)
    , _locked(false)
{
    setRect(0, 0, _defaultWidth, _defaultHeight);

    _lockedGraphicsItem = new IconGraphicsItem(_lockedIcon, this);
    _unlockedGraphicsItem = new IconGraphicsItem(_unlockedIcon, this);

    _scene.addItem(this);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, true);

    _currentFillColor = kUnlockedFillColor;
    _currentBorderColor = kUnselectedBorderColor;

    _borderPen = QPen(_currentBorderColor, 1.0, Qt::PenStyle::DashLine);

    setZValue(-_groupAreaZValue);

    setAcceptHoverEvents(true);
}

GroupGraphicsObject::~GroupGraphicsObject()
{
    _scene.removeItem(this);
}

NodeGroup const &GroupGraphicsObject::group() const
{
    return _group;
}

QRectF GroupGraphicsObject::boundingRect() const
{
    QRectF ret{};

    for (auto nodeId : _group.nodeIds()) {
        auto nodeGraphicsObject = _scene.nodeGraphicsObject(nodeId);

        if (nodeGraphicsObject) {
            ret |= nodeGraphicsObject->mapRectToScene(nodeGraphicsObject->boundingRect());
        }
    }

    if (_possibleChild) {
        auto possibleChildGraphicsObject = _scene.nodeGraphicsObject(_possibleChild);
        if (possibleChildGraphicsObject) {
            ret |= possibleChildGraphicsObject->mapRectToScene(
                possibleChildGraphicsObject->boundingRect());
        }
    }

    return mapRectFromScene(ret.marginsAdded(_margins));
}

void GroupGraphicsObject::setFillColor(const QColor &color)
{
    _currentFillColor = color;
    update();
}

void GroupGraphicsObject::setBorderColor(const QColor &color)
{
    _currentBorderColor = color;
    _borderPen.setColor(_currentBorderColor);
}

void GroupGraphicsObject::moveConnections()
{
    for (auto const &nodeId : group().nodeIds()) {
        auto nodeGraphicsObject = _scene.nodeGraphicsObject(nodeId);

        if (nodeGraphicsObject) {
            nodeGraphicsObject->moveConnections();
        }
    }
}

void GroupGraphicsObject::moveNodes(const QPointF &offset)
{
    for (auto nodeId : group().nodeIds()) {
        auto nodeGraphicsObject = _scene.nodeGraphicsObject(nodeId);

        if (nodeGraphicsObject) {
            nodeGraphicsObject->moveBy(offset.x(), offset.y());
            nodeGraphicsObject->moveConnections();
        }
    }
}

//void GroupGraphicsObject::lock(bool locked)
//{
//  for (auto& connectionInfo : connections())
//  {
//    auto sourceNodeId = std::get<0>(connectionInfo);
//    auto sourcePortIndex = std::get<1>(connectionInfo);
//    auto destNodeId = std::get<2>(connectionInfo);
//    auto destPortIndex = std::get<3>(connectionInfo);

//    auto sourceNodeGraphicsObject = _scene.nodeGraphicsObject(sourceNodeId);
//    auto destNodeGraphicsObject = _scene.nodeGraphicsObject(destNodeId);

//    if (sourceNodeGraphicsObject && destNodeGraphicsObject)
//    {
//        sourceNodeGraphicsObject->setLocked(locked);
//        destNodeGraphicsObject->setLocked(locked);
//    }
//  }

//  _lockedGraphicsItem->setVisible(locked);
//  _unlockedGraphicsItem->setVisible(!locked);
//  setFillColor(locked ? kLockedFillColor : kUnlockedFillColor);
//  _locked = locked;
//  setZValue(locked ? _groupAreaZValue : -_groupAreaZValue);
//}

bool GroupGraphicsObject::locked() const
{
    return _locked;
}

void GroupGraphicsObject::positionLockedIcon()
{
    _lockedGraphicsItem->setPos(
        boundingRect().topRight()
        + QPointF(-(_roundedBorderRadius + IconGraphicsItem::iconSize()), _roundedBorderRadius));
    _unlockedGraphicsItem->setPos(
        boundingRect().topRight()
        + QPointF(-(_roundedBorderRadius + IconGraphicsItem::iconSize()), _roundedBorderRadius));
}

void GroupGraphicsObject::setHovered(bool hovered, QGraphicsSceneHoverEvent *event)
{
    setFillColor(hovered ? (locked() ? kLockedHoverColor : kUnlockedHoverColor)
                         : (locked() ? kLockedFillColor : kUnlockedFillColor));

    for (auto nodeId : _group.nodeIds()) {
        auto nodeGraphicsObject = _scene.nodeGraphicsObject(nodeId);

        auto nodeScene = nodeGraphicsObject->nodeScene();

        Q_EMIT nodeScene->nodeHovered(nodeId, event->screenPos());
    }

    update();
}

void GroupGraphicsObject::setPossibleChild(NodeId possibleChild)
{
    _possibleChild = possibleChild;
}

void GroupGraphicsObject::unsetPossibleChild()
{
    _possibleChild = InvalidNodeId;
}

// TEST THIS NODE
//std::vector<std::tuple<QtNodes::NodeId, QtNodes::PortIndex, QtNodes::NodeId, QtNodes::PortIndex>>
//GroupGraphicsObject::connections() const
//{
//  std::vector<std::tuple<NodeId, PortIndex, NodeId, PortIndex>> result;

//  if (auto dataFlowScene = dynamic_cast<FlowScene*>(&_scene))
//  {
//      // Itera sobre todas as conexões no cenário
//      for (const auto& connection : dataFlowScene->connections())
//      {
//        // Obtém as informações do nó de origem
//        auto sourceNodeId = std::get<0>(connection);
//        auto sourcePortIndex = std::get<1>(connection);

//                // Obtém as informações do nó de destino
//        auto destNodeId = std::get<2>(connection);
//        auto destPortIndex = std::get<3>(connection);

//                // Verifica se ambas as extremidades da conexão estão dentro do grupo
//        if (group().containsNode(sourceNodeId) && group().containsNode(destNodeId))
//        {
//          result.emplace_back(sourceNodeId, sourcePortIndex, destNodeId, destPortIndex);
//        }
//      }
//  }

//  return result;
//}

void GroupGraphicsObject::setPosition(const QPointF &position)
{
    QPointF diffPos = position - scenePos();
    moveNodes(diffPos);
    moveConnections();
}

void GroupGraphicsObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    setHovered(true, event);
}

void GroupGraphicsObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    setHovered(false, event);
}

void GroupGraphicsObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
    if (event->lastPos() != event->pos()) {
        auto diff = event->pos() - event->lastPos();
        moveNodes(diff);
        moveConnections();
    }
}

void GroupGraphicsObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    // TODO
    //  lock(!locked());
}

void GroupGraphicsObject::paint(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                QWidget *widget)
{
    Q_UNUSED(widget);
    prepareGeometryChange();
    setRect(boundingRect());
    positionLockedIcon();
    painter->setClipRect(option->exposedRect);
    painter->setBrush(_currentFillColor);

    setBorderColor(isSelected() ? kSelectedBorderColor : kUnselectedBorderColor);
    painter->setPen(_borderPen);

    painter->drawRoundedRect(rect(), _roundedBorderRadius, _roundedBorderRadius);
}
