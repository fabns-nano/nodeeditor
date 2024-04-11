#include <catch2/catch.hpp>
#include <QtTest>

#include <QtNodes/FlowScene>
#include <QtNodes/FlowView>
// #include <QtNodes/Node>
#include <QtNodes/NodeGroup>

#include "ApplicationSetup.hpp"
#include "StubNodeDataModel.hpp"

using QtNodes::DataFlowGraphicsScene;
using QtNodes::GraphicsView;
using QtNodes::NodeDelegateModelRegistry;
using QtNodes::NodeId;

constexpr size_t nodesPerGroup = 5;
constexpr size_t nGroups = 2;

class MockModel : public StubNodeDataModel
{
public:
    MockModel(int n = 0)
        : StubNodeDataModel()
        , _double(n * 1.9)
        , _int(n)
    {}

    QJsonObject save() const override
    {
        QJsonObject output = NodeDataModel::save();
        output["int"] = _int;
        output["double"] = _double;
        output["caption"] = caption();
        return output;
    }

    void load(const QJsonObject &object) override
    {
        name(object["name"].toString());
        caption(object["caption"].toString());
        _double = object["double"].toDouble();
        _int = object["int"].toInt();
    }

    double _double{};
    int _int{};
};

static std::shared_ptr<NodeDelegateModelRegistry> registerDataModels()
{
    auto ret = std::make_shared<NodeDelegateModelRegistry>();

    ret->registerModel<MockModel>();

    return ret;
}

TEST_CASE("Creating groups from node", "[node groups][namo]")
{
    auto setup = applicationSetup();

    FlowScene scene;
    FlowView view(&scene);

    SECTION("Creating a group from a single node")
    {
        auto &node = scene.createNode(std::make_unique<MockModel>());
        auto nodeID = node.id();

        std::vector<Node *> nodeVec(1, &node);
        scene.createGroup(nodeVec);
        CHECK(scene.groups().size() == 1);
        CHECK(nodeID == scene.groups().begin()++->second->childNodes()[0]->id());
    }

    SECTION("Creating a group from multiple nodes")
    {
        std::vector<std::vector<QUuid>> nodeIDs(nGroups, std::vector<QUuid>(nodesPerGroup));
        std::vector<QUuid> groupIDs(nGroups);

        for (size_t i = 0; i < nGroups; i++) {
            std::vector<Node *> nodes;
            nodes.reserve(nodesPerGroup);
            for (size_t j = 0; j < nodesPerGroup; j++) {
                auto &node = scene.createNode(std::make_unique<MockModel>());
                nodes.push_back(&node);
                nodeIDs[i][j] = node.id();
            }
            auto group_weak = scene.createGroup(nodes);
            if (auto group = group_weak.lock(); group) {
                groupIDs[i] = group->id();
            }
        }

        SECTION("Finding groups in scene via ID")
        {
            for (auto &groupID : groupIDs) {
                auto it = scene.groups().find(groupID);
                CHECK(it != scene.groups().end());
            }
        }

        SECTION("Finding node IDs in groups map")
        {
            for (size_t i = 0; i < nGroups; i++) {
                auto group = scene.groups().at(groupIDs[i]);
                auto ids = group->nodeIDs();

                std::set<QUuid> sceneIdSet;
                sceneIdSet.insert(ids.begin(), ids.end());
                std::set<QUuid> localIdSet;
                localIdSet.insert(nodeIDs[i].begin(), nodeIDs[i].end());

                // checks if the IDs stored in the scene are the same
                // as the ones previously generated.
                CHECK(sceneIdSet == localIdSet);
            }
        }

        SECTION("Checking the group reference of each node")
        {
            for (auto &group : scene.groups()) {
                auto id = group.first;
                for (auto &node : group.second->childNodes()) {
                    auto nodeGroup = node->nodeGroup().lock();
                    CHECK(id == nodeGroup->id());
                }
            }
        }
    }
}

TEST_CASE("Deleting nodes and groups", "[node groups][namo]")
{
    auto setup = applicationSetup();

    FlowScene scene;
    FlowView view(&scene);

    std::vector<std::vector<QUuid>> nodeIDs(nGroups, std::vector<QUuid>(nodesPerGroup));
    std::vector<QUuid> groupIDs(nGroups);

    for (size_t i = 0; i < nGroups; i++) {
        std::vector<Node *> nodes;
        nodes.reserve(nodesPerGroup);
        for (size_t j = 0; j < nodesPerGroup; j++) {
            auto &node = scene.createNode(std::make_unique<MockModel>());
            nodes.push_back(&node);
            nodeIDs[i][j] = node.id();
        }
        auto group_weak = scene.createGroup(nodes);
        if (auto group = group_weak.lock(); group) {
            groupIDs[i] = group->id();
        }
    }

    SECTION("Deleting nodes that belong to a group")
    {
        for (size_t i = 0; i < nGroups; i++) {
            for (size_t j = 0; j < nodesPerGroup - 1; j++) {
                QUuid currentNodeID = nodeIDs[i][j];
                auto &node = scene.nodes().at(currentNodeID);
                if (auto currentGroup = node->nodeGroup().lock(); currentGroup) {
                    scene.removeNode(*node.get());
                    auto ids = currentGroup->nodeIDs();
                    auto nodeInGroupIt = std::find(ids.begin(), ids.end(), currentNodeID);
                    // checks if the id was actually removed from the map
                    CHECK(nodeInGroupIt == ids.end());
                }
            }
        }
    }

    SECTION("Removing a node from a group")
    {
        for (size_t i = 0; i < nGroups; i++) {
            for (size_t j = 0; j < nodesPerGroup - 1; j++) {
                QUuid currentNodeID = nodeIDs[i][j];
                auto &node = scene.nodes().at(currentNodeID);
                if (auto currentGroup = node->nodeGroup().lock(); currentGroup) {
                    currentGroup->removeNode(node.get());
                    auto ids = currentGroup->nodeIDs();

                    // checks if the id was actually removed from the map
                    auto nodeInGroupIt = std::find(ids.begin(), ids.end(), currentNodeID);
                    CHECK(nodeInGroupIt == ids.end());

                    // checks if the node still exists in the scene
                    auto nodeInSceneIt = scene.nodes().find(currentNodeID);
                    CHECK(nodeInSceneIt != scene.nodes().end());

                    SECTION("...then deleting the node")
                    {
                        scene.removeNode(*node.get());
                        ids = currentGroup->nodeIDs();
                        auto nodeInSceneIt = scene.nodes().find(currentNodeID);
                        // checks if the node was removed from the scene
                        CHECK(nodeInSceneIt == scene.nodes().end());
                    }

                    SECTION("...then assigning the node to another group")
                    {
                        std::vector<Node *> nodeVec(1, node.get());
                        auto new_group_weakptr = scene.createGroup(nodeVec);
                        auto newGroup = new_group_weakptr.lock();
                        auto newGroupIDs = newGroup->nodeIDs();
                        auto nodeInGroupIt = std::find(newGroupIDs.begin(),
                                                       newGroupIDs.end(),
                                                       currentNodeID);
                        // checks if the id was included in the new group
                        CHECK(nodeInGroupIt != newGroupIDs.end());
                    }

                    SECTION("...then reassigning the node to the group")
                    {
                        scene.addNodeToGroup(node->id(), currentGroup->id());
                        auto ids = currentGroup->nodeIDs();
                        auto nodeInGroupIt = std::find(ids.begin(), ids.end(), currentNodeID);

                        // checks if the id was actually included in the map
                        CHECK(nodeInGroupIt != ids.end());
                    }
                }
            }
        }
    }

    SECTION("Deleting a whole group")
    {
        for (size_t i = 0; i < nGroups; i++) {
            scene.removeGroup(groupIDs[i]);
            // checks if the group ID was removed from the map
            auto groupIt = scene.groups().find(groupIDs[i]);
            CHECK(groupIt == scene.groups().end());

            for (size_t j = 0; j < nodesPerGroup; j++) {
                // checks if each node was also deleted
                auto nodeIt = scene.nodes().find(nodeIDs[i][j]);
                CHECK(nodeIt == scene.nodes().end());
            }
        }
    }
}

TEST_CASE("Saving and loading groups", "[node groups][namo]")
{
    struct NodeFields
    {
        QUuid id;
        QString name;
        QString caption;
        double doubleVal;
        int intVal;
    };

    auto setup = applicationSetup();

    FlowScene scene(registerDataModels());
    FlowView view(&scene);

    std::unordered_map<QUuid, std::vector<QUuid>> groupNodeMap;
    groupNodeMap.reserve(nGroups);
    std::vector<QUuid> groupIDs(nGroups);

    for (size_t i = 0; i < nGroups; i++) {
        std::vector<Node *> nodes;
        nodes.reserve(nodesPerGroup);

        std::vector<QUuid> nodeIDs;
        nodeIDs.reserve(nodesPerGroup);

        for (size_t j = 0; j < nodesPerGroup; j++) {
            auto &node = scene.createNode(std::make_unique<MockModel>(i * nodesPerGroup + j));
            nodes.push_back(&node);
            nodeIDs[j] = node.id();
        }
        auto group_weak = scene.createGroup(nodes);
        if (auto group = group_weak.lock(); group) {
            groupIDs[i] = group->id();
            groupNodeMap[group->id()] = std::move(nodeIDs);
        }
    }

    SECTION("Saving group file")
    {
        std::vector<QJsonObject> groupFiles;
        groupFiles.reserve(nGroups);

        for (auto &group : scene.groups()) {
            auto groupByteArray = group.second->saveToFile();
            auto groupJsonObj = QJsonDocument::fromJson(groupByteArray).object();
            groupFiles.push_back(groupJsonObj);
        }

        for (auto &groupFile : groupFiles) {
            auto groupIDObj = groupFile["id"];
            CHECK(!groupIDObj.isUndefined());

            auto groupNameObj = groupFile["name"];
            CHECK(!groupNameObj.isUndefined());

            std::vector<NodeFields> restoredNodes;
            restoredNodes.reserve(nodesPerGroup);

            QJsonArray nodesJson = groupFile["nodes"].toArray();
            for (const QJsonValueRef nodeJson : nodesJson) {
                auto nodeObj = nodeJson.toObject();
                NodeFields restoredNode{};

                auto nameObj = nodeObj["name"];
                CHECK(!nameObj.isUndefined());
                restoredNode.name = nameObj.toString();

                auto captionObj = nodeObj["caption"];
                CHECK(!captionObj.isUndefined());
                restoredNode.caption = captionObj.toString();

                auto intObj = nodeObj["int"];
                CHECK(!intObj.isUndefined());
                restoredNode.intVal = intObj.toInt();

                auto doubleObj = nodeObj["double"];
                CHECK(!doubleObj.isUndefined());
                restoredNode.doubleVal = doubleObj.toInt();

                restoredNodes.push_back(restoredNode);
            }

            QUuid groupID{groupIDObj.toString()};
            auto nodeIDs = groupNodeMap[groupID];

            for (auto &nodeID : nodeIDs) {
                for (auto &restoredNode : restoredNodes) {
                    auto nodeEntry = scene.nodes().find(nodeID);
                    CHECK(nodeEntry != scene.nodes().end());

                    auto nodeDataModel = nodeEntry->second->nodeDataModel();
                    auto mockNode = dynamic_cast<MockModel *>(nodeDataModel);
                    CHECK(mockNode);

                    if (restoredNode.id == nodeID) {
                        CHECK(restoredNode.name == nodeEntry->second->nodeDataModel()->name());
                        CHECK(restoredNode.caption == nodeEntry->second->nodeDataModel()->caption());
                        CHECK(qFuzzyCompare(restoredNode.doubleVal, mockNode->_double));
                        CHECK(restoredNode.intVal == mockNode->_int);
                    }
                }
            }
        }
    }
    SECTION("Saving a scene that includes a group")
    {
        std::unordered_map<QUuid, std::vector<NodeFields>> currentSceneMap;
        currentSceneMap.reserve(nGroups);

        // saves the current scene state for comparison
        for (auto &group : scene.groups()) {
            std::vector<NodeFields> sceneNodes;
            sceneNodes.reserve(nodesPerGroup);

            for (auto &node : group.second->childNodes()) {
                auto nodeMockModel = dynamic_cast<MockModel *>(node->nodeDataModel());
                CHECK(nodeMockModel);

                NodeFields nodeFields;
                nodeFields.id = node->id();
                nodeFields.name = nodeMockModel->name();
                nodeFields.caption = nodeMockModel->caption();
                nodeFields.doubleVal = nodeMockModel->_double;
                nodeFields.intVal = nodeMockModel->_int;

                sceneNodes.push_back(nodeFields);
            }

            currentSceneMap[group.first] = std::move(sceneNodes);
        }

        // saves the scene using the actual save function
        auto sceneFile = scene.saveToMemory();
        scene.clearScene();
        CHECK(scene.groups().empty());
        CHECK(scene.nodes().empty());

        // reloads the saved scene and compares its elements with the ones
        // previously stored
        scene.loadFromMemory(sceneFile);
        CHECK(!scene.groups().empty());
        CHECK(!scene.nodes().empty());
        for (const auto &groupEntry : scene.groups()) {
            auto group = groupEntry.second;
            auto &savedNodes = currentSceneMap.at(group->id());
            for (const auto &savedNode : savedNodes) {
                for (const auto &node : group->childNodes()) {
                    if (savedNode.id == node->id()) {
                        auto nodeMockModel = dynamic_cast<MockModel *>(node->nodeDataModel());
                        CHECK(nodeMockModel);
                        CHECK(savedNode.name == nodeMockModel->name());
                        CHECK(savedNode.caption == nodeMockModel->caption());
                        CHECK(qFuzzyCompare(savedNode.doubleVal, nodeMockModel->_double));
                        CHECK(savedNode.intVal == nodeMockModel->_int);
                    }
                }
            }
        }
    }
}
