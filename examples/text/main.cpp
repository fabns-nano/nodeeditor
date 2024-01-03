#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/DataModelRegistry>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>

#include <QtWidgets/QApplication>

#include <QtNodes/DataModelRegistry>

#include "TextDisplayDataModel.hpp"
#include "TextSourceDataModel.hpp"


using QtNodes::DataFlowGraphModel;
using QtNodes::DataModelRegistry;
using QtNodes::FlowScene;
using QtNodes::FlowView;

static std::shared_ptr<DataModelRegistry> registerDataModels() {
  auto ret = std::make_shared<DataModelRegistry>();

  ret->registerModel<TextSourceDataModel>();
  ret->registerModel<TextDisplayDataModel>();

  return ret;
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  std::shared_ptr<DataModelRegistry> registry = registerDataModels();
  DataFlowGraphModel dataFlowGraphModel(registry);

  FlowScene scene(dataFlowGraphModel);

  FlowView view(&scene);

  view.setWindowTitle("Node-based flow editor");
  view.resize(800, 600);
  view.show();

  return app.exec();
}
