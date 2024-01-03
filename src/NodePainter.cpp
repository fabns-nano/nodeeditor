#include "NodePainter.hpp"

#include <cmath>

#include <QtCore/QMargins>

#include "AbstractGraphModel.hpp"
#include "AbstractNodeGeometry.hpp"
#include "BasicGraphicsScene.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "ConnectionIdUtils.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeState.hpp"
#include "StyleCollection.hpp"

namespace QtNodes {

void NodePainter::paint(QPainter* painter, NodeGraphicsObject& ngo) {
  AbstractNodeGeometry& geometry = ngo.nodeScene()->nodeGeometry();

  // TODO TODO TODO TODO
  // geometry.recomputeSizeIfFontChanged(painter->font());

  drawNodeRect(painter, ngo);

  drawConnectionPoints(painter, ngo);

  drawFilledConnectionPoints(painter, ngo);

  drawNodeCaption(painter, ngo);

  drawEntryLabels(painter, ngo);

  drawResizeRect(painter, ngo);
  drawStatusIcon(painter, ngo);
}

void NodePainter::drawNodeRect(QPainter* painter, NodeGraphicsObject& ngo) {
  AbstractGraphModel& model = ngo.graphModel();

  NodeId const nodeId = ngo.nodeId();

  AbstractNodeGeometry& geometry = ngo.nodeScene()->nodeGeometry();

  QSize size = geometry.size(nodeId);

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));

  NodeStyle nodeStyle(json.object());

  auto color = ngo.isSelected() ? nodeStyle.SelectedBoundaryColor
                                : nodeStyle.NormalBoundaryColor;

  auto fill_color0 = ngo.isSelected() ? nodeStyle.SelectedGradientColor0
                                      : nodeStyle.GradientColor0;

  auto fill_color1 = ngo.isSelected() ? nodeStyle.SelectedGradientColor1
                                      : nodeStyle.GradientColor1;

  auto fill_color2 = ngo.isSelected() ? nodeStyle.SelectedGradientColor2
                                      : nodeStyle.GradientColor2;

  auto fill_color3 = ngo.isSelected() ? nodeStyle.SelectedGradientColor3
                                      : nodeStyle.GradientColor3;

  if (ngo.nodeState().hovered()) {
    QPen p(color, nodeStyle.HoveredPenWidth);
    painter->setPen(p);
  } else {
    QPen p(color, nodeStyle.PenWidth);
    painter->setPen(p);
  }

  QLinearGradient gradient(QPointF(0.0, 0.0), QPointF(2.0, size.height()));

  gradient.setColorAt(0.0, fill_color0);
  gradient.setColorAt(0.03, fill_color1);
  gradient.setColorAt(0.97, fill_color2);
  gradient.setColorAt(1.0, fill_color3);

  painter->setBrush(gradient);

  float diam = nodeStyle.ConnectionPointDiameter;

  QRectF boundary(-diam, -diam, 2.0 * diam + size.width(),
                  2.0 * diam + size.height());

  double const radius = 3.0;

  painter->drawRoundedRect(boundary, radius, radius);
}

void NodePainter::drawConnectionPoints(QPainter* painter,
                                       NodeGraphicsObject& ngo) {
  AbstractGraphModel& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  AbstractNodeGeometry& geometry = ngo.nodeScene()->nodeGeometry();

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));
  NodeStyle nodeStyle(json.object());

  auto const& connectionStyle = StyleCollection::connectionStyle();

  float diameter = nodeStyle.ConnectionPointDiameter;
  auto reducedDiameter = diameter * 0.8;

  for (PortType portType : {PortType::Out, PortType::In}) {
    size_t const n = model
                         .nodeData(nodeId, (portType == PortType::Out)
                                               ? NodeRole::OutPortCount
                                               : NodeRole::InPortCount)
                         .toUInt();

    for (PortIndex portIndex = 0; portIndex < n; ++portIndex) {
      QPointF p = geometry.portPosition(nodeId, portType, portIndex);

      auto const& dataType =
          model.portData(nodeId, portType, portIndex, PortRole::DataType)
              .value<NodeDataType>();

      double r = 1.0;

      NodeState const& state = ngo.nodeState();

      if (auto const* cgo = state.connectionForReaction()) {
        PortType requiredPort = cgo->connectionState().requiredPort();

        if (requiredPort == portType) {
          ConnectionId possibleConnectionId =
              makeCompleteConnectionId(cgo->connectionId(), nodeId, portIndex);

          bool const possible = model.connectionPossible(possibleConnectionId);

          auto cp = cgo->sceneTransform().map(cgo->endPoint(requiredPort));
          cp = ngo.sceneTransform().inverted().map(cp);

          auto diff = cp - p;
          double dist = std::sqrt(QPointF::dotProduct(diff, diff));

          if (possible) {
            double const thres = 40.0;
            r = (dist < thres) ? (2.0 - dist / thres) : 1.0;
          } else {
            double const thres = 80.0;
            r = (dist < thres) ? (dist / thres) : 1.0;
          }
        }
      }

      if (connectionStyle.useDataDefinedColors()) {
        painter->setBrush(connectionStyle.normalColor(dataType.id));
      } else {
        painter->setBrush(nodeStyle.ConnectionPointColor);
      }

      painter->drawEllipse(p, reducedDiameter * r, reducedDiameter * r);
    }
  }

  if (ngo.nodeState().connectionForReaction()) {
    ngo.nodeState().resetConnectionForReaction();
  }
}

void NodePainter::drawFilledConnectionPoints(QPainter* painter,
                                             NodeGraphicsObject& ngo) {
  AbstractGraphModel& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  AbstractNodeGeometry& geometry = ngo.nodeScene()->nodeGeometry();

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));
  NodeStyle nodeStyle(json.object());

  auto diameter = nodeStyle.ConnectionPointDiameter;

  for (PortType portType : {PortType::Out, PortType::In}) {
    size_t const n = model
                         .nodeData(nodeId, (portType == PortType::Out)
                                               ? NodeRole::OutPortCount
                                               : NodeRole::InPortCount)
                         .toUInt();

    for (PortIndex portIndex = 0; portIndex < n; ++portIndex) {
      QPointF p = geometry.portPosition(nodeId, portType, portIndex);

      auto const& connected = model.connections(nodeId, portType, portIndex);

      if (!connected.empty()) {
        auto const& dataType =
            model.portData(nodeId, portType, portIndex, PortRole::DataType)
                .value<NodeDataType>();

        auto const& connectionStyle = StyleCollection::connectionStyle();
        if (connectionStyle.useDataDefinedColors()) {
          QColor const c = connectionStyle.normalColor(dataType.id);
          painter->setPen(c);
          painter->setBrush(c);
        } else {
          painter->setPen(nodeStyle.FilledConnectionPointColor);
          painter->setBrush(nodeStyle.FilledConnectionPointColor);
        }

        painter->drawEllipse(p, diameter * 0.8, diameter * 0.8);
      }
    }
  }
}

void NodePainter::drawNodeCaption(QPainter* painter, NodeGraphicsObject& ngo) {
  AbstractGraphModel& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  AbstractNodeGeometry& geometry = ngo.nodeScene()->nodeGeometry();

  if (!model.nodeData(nodeId, NodeRole::CaptionVisible).toBool())
    return;

  QString const name = model.nodeData(nodeId, NodeRole::Caption).toString();

  QFont font = painter->font();
  font.setBold(!model.nodeData(nodeId, NodeRole::NicknameVisible).toBool());
  font.setItalic(model.nodeData(nodeId, NodeRole::NicknameVisible).toBool());

  QPointF position = geometry.captionPosition(nodeId);

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));
  NodeStyle nodeStyle(json.object());

  painter->setFont(font);
  painter->setPen(nodeStyle.FontColor);
  painter->drawText(position, name);

  font.setBold(false);
  font.setItalic(false);
  painter->setFont(font);
}

void NodePainter::drawEntryLabels(QPainter* painter, NodeGraphicsObject& ngo) {
  AbstractGraphModel& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  AbstractNodeGeometry& geometry = ngo.nodeScene()->nodeGeometry();

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));
  NodeStyle nodeStyle(json.object());

  for (PortType portType : {PortType::Out, PortType::In}) {
    unsigned int n = model.nodeData<unsigned int>(
        nodeId, (portType == PortType::Out) ? NodeRole::OutPortCount
                                            : NodeRole::InPortCount);

    for (PortIndex portIndex = 0; portIndex < n; ++portIndex) {
      auto const& connected = model.connections(nodeId, portType, portIndex);

      QPointF p = geometry.portTextPosition(nodeId, portType, portIndex);

      if (connected.empty())
        painter->setPen(nodeStyle.FontColorFaded);
      else
        painter->setPen(nodeStyle.FontColor);

      QString s;

      if (model.portData<bool>(nodeId, portType, portIndex,
                               PortRole::CaptionVisible)) {
        s = model.portData<QString>(nodeId, portType, portIndex,
                                    PortRole::Caption);
      } else {
        auto portData =
            model.portData(nodeId, portType, portIndex, PortRole::DataType);

        s = portData.value<NodeDataType>().name;
      }

      painter->drawText(p, s);
    }
  }
}

void NodePainter::drawResizeRect(QPainter* painter, NodeGraphicsObject& ngo) {
  AbstractGraphModel& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  AbstractNodeGeometry& geometry = ngo.nodeScene()->nodeGeometry();

  if (model.nodeFlags(nodeId) & NodeFlag::Resizable) {
    painter->setBrush(Qt::gray);

    painter->drawEllipse(geometry.resizeHandleRect(nodeId));
  }
}

void NodePainter::drawStatusIcon(QPainter* painter,
                                 NodeGraphicsObject const& ngo) {
  NodeGeometry geom(ngo);
  geom.updateStatusIconSize();
  painter->drawPixmap(geom.statusIconRect(), geom.processingStatusIcon().pixmap(
                                                 geom.statusIconSize() * 3));
}

void NodePainter::drawProgressValue(QPainter* painter,
                                    NodeGraphicsObject const& ngo,
                                    QString const& nodeProgress) {
  GraphModel const& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  NodeGeometry geom(ngo);

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));
  NodeStyle nodeStyle(json);
  QFont font = painter->font();
  font.setBold(true);

  auto rect = QFontMetrics(font).boundingRect(nodeProgress);

  // TODO: check if size.height() = to geo.height()
  QSize size = model.nodeData(nodeId, NodeRole::Size).value<QSize>();
  QPointF position(rect.width() / 2.0, size.height() - rect.height());

  painter->setFont(font);
  painter->setPen(nodeStyle.FontColor);
  painter->drawText(position, nodeProgress);

  font.setBold(false);
  painter->setFont(font);
}

}  // namespace QtNodes
