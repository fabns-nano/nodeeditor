#include "NodePainter.hpp"

#include <cmath>

#include <QtCore/QMargins>

#include "AbstractGraphModel.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "ConnectionIdUtils.hpp"
#include "NodeGeometry.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeState.hpp"
#include "StyleCollection.hpp"

namespace QtNodes {

void NodePainter::paint(QPainter* painter, NodeGraphicsObject& ngo) {
  NodeGeometry geometry(ngo);
  GraphModel const& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();

  geometry.recalculateSizeIfFontChanged(painter->font());

  drawNodeRect(painter, ngo);

  drawConnectionPoints(painter, ngo);

  drawFilledConnectionPoints(painter, ngo);

  drawNodeCaption(painter, ngo);

  drawEntryLabels(painter, ngo);

  drawResizeRect(painter, ngo);

  drawFilledConnectionPoints(painter, ngo);

  drawNodeCaption(painter, ngo);

  drawEntryLabels(painter, ngo);

  drawResizeRect(painter, ngo);

  drawStatusIcon(painter, ngo);

  drawResizeRect(painter, ngo);

  if (!model.nodeData(nodeId, NodeRole::CaptionVisible).toBool()) {
    drawModelNickname(painter, ngo);
    drawNodeCaption(painter, ngo);

    GraphModel const& model = ngo.graphModel();
    auto nodeId = ngo.nodeId();

    auto processingStatusVariant =
        model.nodeData(nodeId, NodeRole::ProcessingStatus);
    NodeProcessingStatus processingStatus =
        static_cast<NodeProcessingStatus>(processingStatusVariant.toInt());

    if (processingStatus == NodeProcessingStatus::Processing) {
      if (!model.nodeData(nodeId, NodeRole::ProgressValue)
               .toString()
               .isNull()) {
        drawProgressValue(
            painter, ngo,
            model.nodeData(nodeId, NodeRole::ProgressValue).toString());
      }
    }
  }

  /// call custom painter
  // TODO: think about and implement custom painter delegate
  // if (auto painterDelegate = model->painterDelegate())
  //{
  // painterDelegate->paint(painter, geom, model);
  //}
}

void NodePainter::drawNodeRect(QPainter* painter,
                               NodeGraphicsObject const& ngo) {
  AbstractGraphModel const& model = ngo.graphModel();

  NodeId const nodeId = ngo.nodeId();

  NodeGeometry geom(ngo);
  QSize size = geom.size();

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));

  NodeStyle nodeStyle(json);

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
  AbstractGraphModel const& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  NodeGeometry geom(ngo);

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));
  NodeStyle nodeStyle(json);

  auto const& connectionStyle = StyleCollection::connectionStyle();

  float diameter = nodeStyle.ConnectionPointDiameter;
  auto reducedDiameter = diameter * 0.8;

  for (PortType portType : {PortType::Out, PortType::In}) {
    size_t const n = model
                         .nodeData(nodeId, (portType == PortType::Out)
                                               ? NodeRole::NumberOfOutPorts
                                               : NodeRole::NumberOfInPorts)
                         .toUInt();

    for (PortIndex portIndex = 0; portIndex < n; ++portIndex) {
      QPointF p = geom.portNodePosition(portType, portIndex);

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
  AbstractGraphModel const& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  NodeGeometry geom(ngo);

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));
  NodeStyle nodeStyle(json);

  auto diameter = nodeStyle.ConnectionPointDiameter;

  for (PortType portType : {PortType::Out, PortType::In}) {
    size_t const n = model
                         .nodeData(nodeId, (portType == PortType::Out)
                                               ? NodeRole::NumberOfOutPorts
                                               : NodeRole::NumberOfInPorts)
                         .toUInt();

    for (PortIndex portIndex = 0; portIndex < n; ++portIndex) {
      QPointF p = geom.portNodePosition(portType, portIndex);

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
  AbstractGraphModel const& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  NodeGeometry geom(ngo);
  QSize size = geom.size();

  if (!model.nodeData(nodeId, NodeRole::CaptionVisible).toBool())
    return;

  QString const name = model.nodeData(nodeId, NodeRole::Caption).toString();

  QFont font = painter->font();
  font.setBold(!model.nodeData(nodeId, NodeRole::NicknameVisible).toBool());
  font.setItalic(model.nodeData(nodeId, NodeRole::NicknameVisible).toBool());

  QFontMetrics metrics(font);
  auto rect = metrics.boundingRect(name);

  int nicknameOffset =
      model.nodeData(nodeId, NodeRole::NicknameVisible).toBool() ? rect.height()
                                                                 : 0;

  double yPos =
      (geom.verticalSpacing() + geom.entryHeight() + nicknameOffset) / 3.0;
  if (model.nodeData(nodeId, NodeRole::NicknameVisible).toBool())
    yPos += 2.0 * geom.verticalSpacing() / 3.0;

  QPointF position((size.width() - rect.width()) / 2.0, yPos);

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));
  NodeStyle nodeStyle(json);

  painter->setFont(font);
  painter->setPen(nodeStyle.FontColor);
  painter->drawText(position, name);

  font.setBold(false);
  font.setItalic(false);
  painter->setFont(font);
}

void NodePainter::drawEntryLabels(QPainter* painter, NodeGraphicsObject& ngo) {
  AbstractGraphModel const& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  NodeGeometry geom(ngo);

  QJsonDocument json =
      QJsonDocument::fromVariant(model.nodeData(nodeId, NodeRole::Style));
  NodeStyle nodeStyle(json);

  QSize size = geom.size();

  for (PortType portType : {PortType::Out, PortType::In}) {
    size_t const n = model
                         .nodeData(nodeId, (portType == PortType::Out)
                                               ? NodeRole::NumberOfOutPorts
                                               : NodeRole::NumberOfInPorts)
                         .toUInt();

    for (PortIndex portIndex = 0; portIndex < n; ++portIndex) {
      auto const& connected = model.connections(nodeId, portType, portIndex);

      QPointF p = geom.portNodePosition(portType, portIndex);

      if (connected.empty())
        painter->setPen(nodeStyle.FontColorFaded);
      else
        painter->setPen(nodeStyle.FontColor);

      QString s;

      if (model.portData(nodeId, portType, portIndex, PortRole::CaptionVisible)
              .toBool()) {
        s = model.portData(nodeId, portType, portIndex, PortRole::Caption)
                .toString();
      } else {
        auto portData =
            model.portData(nodeId, portType, portIndex, PortRole::DataType);

        s = portData.value<NodeDataType>().name;
      }

      QFontMetrics const& metrics = painter->fontMetrics();
      auto rect = metrics.boundingRect(s);

      p.setY(p.y() + rect.height() / 4.0);

      switch (portType) {
        case PortType::In:
          p.setX(5.0);
          break;

        case PortType::Out:
          p.setX(size.width() - 5.0 - rect.width());
          break;

        default:
          break;
      }

      painter->drawText(p, s);
    }
  }
}

void NodePainter::drawResizeRect(QPainter* painter, NodeGraphicsObject& ngo) {
  AbstractGraphModel const& model = ngo.graphModel();
  NodeId const nodeId = ngo.nodeId();
  NodeGeometry geom(ngo);

  if (model.nodeFlags(nodeId) & NodeFlag::Resizable) {
    painter->setBrush(Qt::gray);

    painter->drawEllipse(geom.resizeRect());
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
