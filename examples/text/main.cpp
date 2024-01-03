#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>


#include <QtWidgets/QApplication>

#include <QtNodes/DataModelRegistry>

#include "TextDisplayDataModel.hpp"
#include "TextSourceDataModel.hpp"

using QtNodes::DataFlowGraphicsScene;
using QtNodes::DataFlowGraphModel;
using QtNodes::GraphicsView;
using QtNodes::NodeDelegateModelRegistry;

static std::shared_ptr<NodeDelegateModelRegistry> registerDataModels() {
  auto ret = std::make_shared<NodeDelegateModelRegistry>();

  ret->registerModel<TextSourceDataModel>();
  ret->registerModel<TextDisplayDataModel>();

  return ret;
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();
  DataFlowGraphModel dataFlowGraphModel(registry);

  FlowScene scene(dataFlowGraphModel);

  FlowView view(&scene);

  view.setWindowTitle("Node-based flow editor");
  view.resize(800, 600);
  view.show();

  return app.exec();
}
