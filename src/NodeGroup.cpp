// NodeGroup.cpp

#include "NodeGroup.hpp"
//#include "Node.hpp"
#include "GroupGraphicsObject.hpp" // Certifique-se de incluir o cabeçalho correto
#include "qjsonarray.h"

using QtNodes::GroupGraphicsObject;
using QtNodes::NodeGroup;

int NodeGroup::_groupCount = 0;

NodeGroup::NodeGroup(QVector<NodeId> nodeIds,
                     const QUuid& uid,
                     QString name ,
                     QObject* parent)
    : QObject(parent),
      _name(std::move(name)),
      _uid(uid),
      _nodeIds(std::move(nodeIds)),
      _groupGraphicsObject(nullptr) {
  _groupCount++;
}

QByteArray NodeGroup::saveToFile() const {
  QJsonObject groupJson;

  groupJson["name"] = _name;
  groupJson["id"] = QUuid(_uid).toString();


  QJsonArray nodesJson;
  for (const auto& nodeId : _nodeIds) {
    QJsonObject nodeJson;
    nodeJson["id"] = static_cast<int>(nodeId);
    nodesJson.append(nodeJson);
  }
  groupJson["nodes"] = nodesJson;

  QJsonDocument groupDocument(groupJson);

  return groupDocument.toJson();
}

QUuid NodeGroup::id() const {
  return _uid;
}

GroupGraphicsObject& NodeGroup::groupGraphicsObject() {
  return *_groupGraphicsObject;
}


const QString& NodeGroup::name() const {
  return _name;
}

void NodeGroup::setGraphicsObject(std::unique_ptr<GroupGraphicsObject>&& graphics_object) {
  _groupGraphicsObject = std::move(graphics_object);
  // TODO
  //  _groupGraphicsObject->lock(true);
}

bool NodeGroup::empty() const {
  return _nodeIds.empty();
}

int NodeGroup::groupCount() {
  return _groupCount;
}

void NodeGroup::addNode(NodeId nodeId) {
 _nodeIds.push_back(nodeId);
}

void NodeGroup::removeNode(NodeId nodeId) {
 auto it = std::find(_nodeIds.begin(), _nodeIds.end(), nodeId);
 if (it != _nodeIds.end()) {
    _nodeIds.erase(it);
    if(_groupGraphicsObject != nullptr){
      groupGraphicsObject().positionLockedIcon();
    }
 }
}


bool NodeGroup::containsNode(NodeId nodeId) const {
  return _nodeIds.contains(nodeId);
}


QVector<QtNodes::NodeId> NodeGroup::nodeIds() const
{
  return _nodeIds;
}
