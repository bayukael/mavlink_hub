#include <mavlink_hub/app/App.h>

int main(int argc, char* argv[])
{
  using App = pendarlab::app::mavlink_hub::App;
  App app(argc, argv);
  auto result = app.run();
  return result.success? 0 : -1;
}