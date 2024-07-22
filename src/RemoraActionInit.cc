#include "RemoraActionInit.hh"

void RemoraActionInit::Build() const {
  previousActionInit_->Build();

  // build the remora tracking action
  SetUserAction(new RemoraTrackingAction());
}

void RemoraActionInit::BuildForMaster() const {
  previousActionInit_->BuildForMaster();
}