#include "app/App.h"

int main(int argc, char* argv[])
{
  using App = pendarlab::app::mavlink_hub::App;
  App app(argc, argv);
  int result = app.run();
  return result;
}