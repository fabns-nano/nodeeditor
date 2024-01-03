#include <QtWidgets/QApplication>

#include <QtNodes/ConnectionStyle>
#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>


#include "models.hpp"

using QtNodes::DataFlowGraphModel;
using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::FlowView;
using QtNodes::NodeDelegateModelRegistry;

static std::shared_ptr<NodeDelegateModelRegistry> registerDataModels() {
  auto ret = std::make_shared<NodeDelegateModelRegistry>();

  ret->registerModel<NaiveDataModel>();

  /*
     We could have more models registered.
     All of them become items in the context meny of the scene.

     ret->registerModel<AnotherDataModel>();
     ret->registerModel<OneMoreDataModel>();

   */

  return ret;
}

static void setStyle() {
  ConnectionStyle::setConnectionStyle(
      R"(
  {
    "ConnectionStyle": {
      "UseDataDefinedColors": true
    }
  }
  )");
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  setStyle();

  std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();
  DataFlowGraphModel dataFlowGraphModel(registry);

  FlowScene scene(dataFlowGraphModel);

  FlowView view(&scene);

  view.setWindowTitle("Node-based flow editor");
  view.resize(800, 600);
  view.show();

  return app.exec();
}
