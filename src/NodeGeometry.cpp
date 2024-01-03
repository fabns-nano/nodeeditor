#include "NodeGeometry.hpp"

#include "AbstractGraphModel.hpp"
#include "Definitions.hpp"
#include "NodeGraphicsObject.hpp"
// #include "NodeState.hpp"
// #include "PortType.hpp"
#include "NodeDataModel.hpp"
#include "StyleCollection.hpp"

#include <QtWidgets/QWidget>

// #include <QtGlobal>

#include <cmath>
#include <iostream>

namespace QtNodes {

NodeGeometry::NodeGeometry(NodeGraphicsObject const& ngo)
    : _ngo(ngo),
      _graphModel(ngo.graphModel()),
      _defaultInPortWidth(70),
      _defaultOutPortWidth(70),
      _entryHeight(20),
      _verticalSpacing(20),
      _fontMetrics(QFont()),
      _boldFontMetrics(QFont()) {
  QFont f;
  f.setBold(true);

  _boldFontMetrics = QFontMetrics(f);

  _entryHeight = _fontMetrics.height();

  NodeId nodeId = _ngo.nodeId();
  GraphModel const& model = ngo.graphModel();

  auto processingStatusVariant =
      model.nodeData(nodeId, NodeRole::ProcessingStatus);
  NodeProcessingStatus processingStatus =
      static_cast<NodeProcessingStatus>(processingStatusVariant.toInt());
  _statusIconActive = processingStatus != NodeProcessingStatus::NoStatus;

  model.nodeData(nodeId, NodeRole::ProcessingStatus);

  _statusIconSize.setWidth(_statusIconActive ? 32 : 0);
  _statusIconSize.setHeight(_statusIconActive ? 32 : 0);

  _nSources = _graphModel.nodeData(nodeId, NodeRole::NumberOfInPorts).toInt();
  _nSinks = _graphModel.nodeData(nodeId, NodeRole::NumberOfOutPorts).toInt();
}

unsigned int NodeGeometry::entryHeight() const {
  return _entryHeight;
}

unsigned int NodeGeometry::verticalSpacing() const {
  return _verticalSpacing;
}

// QRectF
// NodeGeometry::
// entryBoundingRect() const
//{
// double const addon = 0.0;

// return QRectF(0 - addon,
// 0 - addon,
//_entryWidth + 2 * addon,
//_entryHeight + 2 * addon);
// }

QRectF NodeGeometry::boundingRect() const {
  NodeId nodeId = _ngo.nodeId();

  auto const& style = _graphModel.nodeData(nodeId, NodeRole::Style);
  auto const& nodeStyle = StyleCollection::nodeStyle();

  float addon = 2 * nodeStyle.ConnectionPointDiameter;

  QSize size =
      _graphModel.nodeData(_ngo.nodeId(), NodeRole::Size).value<QSize>();

  return QRectF(0 - addon, 0 - addon, size.width() + 2 * addon,
                size.height() + 2 * addon);
}

QSize NodeGeometry::size() const {
  QSize size =
      _graphModel.nodeData(_ngo.nodeId(), NodeRole::Size).value<QSize>();

  return size;
}

QSize NodeGeometry::recalculateSize() const {
  NodeId nodeId = _ngo.nodeId();

  unsigned int height = 0;
  {
    unsigned int nInPorts =
        _graphModel.nodeData(nodeId, NodeRole::NumberOfInPorts).toUInt();

    unsigned int nOutPorts =
        _graphModel.nodeData(nodeId, NodeRole::NumberOfOutPorts).toUInt();

    unsigned int maxNumOfEntries = std::max(nInPorts, nOutPorts);
    unsigned int step = _entryHeight + _verticalSpacing;
    height = step * maxNumOfEntries + _statusIconSize.height();
    ;
  }

  if (auto w =
          _graphModel.nodeData(nodeId, NodeRole::Widget).value<QWidget*>()) {
    height = std::max(height, static_cast<unsigned int>(w->height()));
  }

  height += captionHeight() + nicknameHeight();

  unsigned int inPortWidth = portWidth(PortType::In);
  unsigned int outPortWidth = portWidth(PortType::Out);

  unsigned int width =
      std::max(inPortWidth + outPortWidth,
               static_cast<unsigned int>(_statusIconSize.width())) +
      2 * _verticalSpacing;

  if (auto w =
          _graphModel.nodeData(nodeId, NodeRole::Widget).value<QWidget*>()) {
    width += w->width();
  }

  width = std::max(width, captionWidth());
  width = std::max(width, nicknameWidth());

  QSize size(width, height);

  _graphModel.setNodeData(_ngo.nodeId(), NodeRole::Size, size);

  return size;
}

QSize NodeGeometry::recalculateSizeIfFontChanged(QFont const& font) const {
  QFontMetrics fontMetrics(font);
  QFont boldFont = font;

  boldFont.setBold(true);

  QFontMetrics boldFontMetrics(boldFont);

  if (_boldFontMetrics != boldFontMetrics) {
    _fontMetrics = fontMetrics;
    _boldFontMetrics = boldFontMetrics;
  }

  return recalculateSize();
}

QPointF NodeGeometry::portNodePosition(PortType const portType,
                                       PortIndex const index) const {
  auto const& nodeStyle = StyleCollection::nodeStyle();

  unsigned int step = _entryHeight + _verticalSpacing;

  QPointF result;

  double totalHeight = 0.0;

  totalHeight += captionHeight() + nicknameHeight();

  totalHeight += step * index;

  // TODO: why?
  totalHeight += step / 2.0;

  QSize size =
      _graphModel.nodeData(_ngo.nodeId(), NodeRole::Size).value<QSize>();

  switch (portType) {
    case PortType::Out: {
      double x = size.width() + nodeStyle.ConnectionPointDiameter;

      result = QPointF(x, totalHeight);
      break;
    }

    case PortType::In: {
      double x = 0.0 - nodeStyle.ConnectionPointDiameter;

      result = QPointF(x, totalHeight);
      break;
    }

    default:
      break;
  }

  return result;
}

QPointF NodeGeometry::portScenePosition(PortType const portType,
                                        PortIndex const index,
                                        QTransform const& t) const {
  QPointF result = portNodePosition(portType, index);

  return t.map(result);
}

// TODO check implementation
PortIndex NodeGeometry::checkHitScenePoint(
    PortType portType,
    QPointF const scenePoint,
    QTransform const& sceneTransform) const {
  auto const& nodeStyle = StyleCollection::nodeStyle();

  PortIndex result = InvalidPortIndex;

  if (portType == PortType::None)
    return result;

  double const tolerance = 2.0 * nodeStyle.ConnectionPointDiameter;

  NodeId nodeId = _ngo.nodeId();

  size_t const n = _graphModel
                       .nodeData(nodeId, (portType == PortType::Out)
                                             ? NodeRole::NumberOfOutPorts
                                             : NodeRole::NumberOfInPorts)
                       .toUInt();

  for (unsigned int portIndex = 0; portIndex < n; ++portIndex) {
    auto pp = portScenePosition(portType, portIndex, sceneTransform);

    QPointF p = pp - scenePoint;
    auto distance = std::sqrt(QPointF::dotProduct(p, p));

    if (distance < tolerance) {
      result = portIndex;
      break;
    }
  }

  return result;
}

QRect NodeGeometry::resizeRect() const {
  QSize size =
      _graphModel.nodeData(_ngo.nodeId(), NodeRole::Size).value<QSize>();

  unsigned int rectSize = 7;

  return QRect(size.width() - rectSize, size.height() - rectSize, rectSize,
               rectSize);
}

QPointF NodeGeometry::widgetPosition() const {
  QSize size =
      _graphModel.nodeData(_ngo.nodeId(), NodeRole::Size).value<QSize>();

  NodeId const nodeId = _ngo.nodeId();
  if (auto w =
          _graphModel.nodeData(nodeId, NodeRole::Widget).value<QWidget*>()) {
    // If the widget wants to use as much vertical space as possible,
    // place it immediately after the caption.
    if (w->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag) {
      return QPointF(_verticalSpacing + portWidth(PortType::In),
                     captionHeight() + nicknameHeight());
    } else {
      return QPointF(
          _verticalSpacing + portWidth(PortType::In),
          (captionHeight() + nicknameHeight() + size.height() - w->height()) /
              2.0);
    }
  }
  return QPointF();
}

int NodeGeometry::equivalentWidgetHeight() const {
  QSize size =
      _graphModel.nodeData(_ngo.nodeId(), NodeRole::Size).value<QSize>();
  // if (_dataModel->validationState() != NodeValidationState::Valid)
  //{
  // return height() - captionHeight() + validationHeight();
  // }
  return size.height() - captionHeight() - nicknameHeight();
}

unsigned int NodeGeometry::captionHeight() const {
  NodeId nodeId = _ngo.nodeId();
  GraphModel const& model = _ngo.graphModel();

  if (!_graphModel.nodeData(nodeId, NodeRole::CaptionVisible).toBool())
    return 0;

  QString name = _graphModel.nodeData(nodeId, NodeRole::Caption).toString();

  auto nicknameVisible =
      model.nodeData(nodeId, NodeRole::NicknameVisible).toBool();

  return nicknameVisible ? _fontMetrics.boundingRect(name).height()
                         : _boldFontMetrics.boundingRect(name).height();
}

unsigned int NodeGeometry::captionWidth() const {
  NodeId nodeId = _ngo.nodeId();

  if (!_graphModel.nodeData(nodeId, NodeRole::CaptionVisible).toBool())
    return 0;

  QString name = _graphModel.nodeData(nodeId, NodeRole::Caption).toString();

  return _boldFontMetrics.boundingRect(name).width();
}

// unsigned int
// NodeGeometry::
// validationHeight() const
//{
// QString msg = _dataModel->validationMessage();

// return _boldFontMetrics.boundingRect(msg).height();
// }

// unsigned int
// NodeGeometry::
// validationWidth() const
//{
// QString msg = _dataModel->validationMessage();

// return _boldFontMetrics.boundingRect(msg).width();
// }

// QPointF
// NodeGeometry::
// calculateNodePositionBetweenNodePorts(PortIndex targetPortIndex, PortType
// targetPort, Node * targetNode, PortIndex sourcePortIndex, PortType
// sourcePort, Node * sourceNode, Node & newNode)
//{
////Calculating the nodes position in the scene. It'll be positioned half way
////between the two ports that it "connects".  The first line calculates the
////halfway point between the ports (node position + port position on the node
////for both nodes averaged).  The second line offsets this coordinate with the
////size of the new node, so that the new nodes center falls on the originally
////calculated coordinate, instead of it's upper left corner.
// auto converterNodePos = (sourceNode->nodeGraphicsObject().pos() +
// sourceNode->nodeGeometry().portScenePosition(sourcePortIndex, sourcePort) +
// targetNode->nodeGraphicsObject().pos() +
// targetNode->nodeGeometry().portScenePosition(targetPortIndex, targetPort))
// / 2.0f; converterNodePos.setX(converterNodePos.x() -
// newNode.nodeGeometry().width() / 2.0f);
// converterNodePos.setY(converterNodePos.y() - newNode.nodeGeometry().height()
// / 2.0f); return converterNodePos;
// }

void NodeGeometry::updateStatusIconSize() const {
  bool oldStatus = _statusIconActive;

  GraphModel const& model = _ngo.graphModel();
  auto nodeId = _ngo.nodeId();

  auto processingStatusVariant =
      model.nodeData(nodeId, NodeRole::ProcessingStatus);
  NodeProcessingStatus processingStatus =
      static_cast<NodeProcessingStatus>(processingStatusVariant.toInt());

  _statusIconActive = processingStatus != NodeProcessingStatus::NoStatus;

  if (oldStatus != _statusIconActive) {
    _statusIconSize.setWidth(_statusIconActive ? 32 : 0);
    _statusIconSize.setHeight(_statusIconActive ? 32 : 0);
  }
}

QSize NodeGeometry::statusIconSize() const {
  return _statusIconSize;
}

QRect NodeGeometry::statusIconRect() const {
  int maxSourcesSinks = static_cast<int>(std::max(_nSources, _nSinks));

  auto iconPos =
      portScenePosition(PortType::Out, maxSourcesSinks).toPoint() +
      QPoint{static_cast<int>(-statusIconSize().width() - _verticalSpacing / 2),
             0};

  return QRect{iconPos, statusIconSize()};
}

const QIcon& NodeGeometry::processingStatusIcon() const {
  GraphModel const& model = _ngo.graphModel();
  auto nodeId = _ngo.nodeId();
  auto processingStatusVariant =
      model.nodeData(nodeId, NodeRole::ProcessingStatus);
  NodeProcessingStatus processingStatus =
      static_cast<NodeProcessingStatus>(processingStatusVariant.toInt());

  switch (processingStatus) {
    case NodeProcessingStatus::NoStatus:
    case NodeProcessingStatus::Updated:
      return _statusUpdated;
    case NodeProcessingStatus::Processing:
      return _statusProcessing;
    case NodeProcessingStatus::Pending:
      return _statusPending;
    case NodeProcessingStatus::Empty:
      return _statusEmpty;
    case NodeProcessingStatus::Failed:
      return _statusInvalid;
    case NodeProcessingStatus::Partial:
      return _statusPartial;
  }
  return _statusInvalid;
}

unsigned int NodeGeometry::portWidth(PortType portType) const {
  unsigned width = 0;

  NodeId nodeId = _ngo.nodeId();

  size_t const n = _graphModel
                       .nodeData(nodeId, (portType == PortType::Out)
                                             ? NodeRole::NumberOfOutPorts
                                             : NodeRole::NumberOfInPorts)
                       .toUInt();

  for (PortIndex portIndex = 0ul; portIndex < n; ++portIndex) {
    QString name;

    if (_graphModel
            .portData(nodeId, portType, portIndex, PortRole::CaptionVisible)
            .toBool()) {
      QVariant portData =
          _graphModel.portData(nodeId, portType, portIndex, PortRole::Caption);

      name = portData.toString();
    } else {
      QVariant portData =
          _graphModel.portData(nodeId, portType, portIndex, PortRole::DataType);
      name = portData.value<NodeDataType>().name;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    width = std::max(unsigned(_fontMetrics.horizontalAdvance(name)), width);
#else
    width = std::max(unsigned(_fontMetrics.width(name)), width);
#endif
  }

  return width;
}

}  // namespace QtNodes
