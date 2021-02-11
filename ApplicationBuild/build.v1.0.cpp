#include "main/model.h"
#include "main/rayTracer.h"
#include "main/simulator.h"
#include "obj/generators.h"

#include <algorithm>

int main() {

  float modelSize = 1.0;
  float sourcePower = 500;
  float frequency = 1e3;

  int numOfCollectors = 37;
  int numOfRaysAlongEachAxis = 9;

  Model model = Model::NewReferenceModel(modelSize);
  RayTracer rayTracer(&model);
  PositionTracker positionTracker("./data");
  generators::PointSpeakerRayFactory pointSpeaker(numOfRaysAlongEachAxis,
                                                  sourcePower, &model);
  generators::FakeOffseter rayOffseter;
  Simulator simulator(&rayTracer, &model, &pointSpeaker, &rayOffseter,
                      &positionTracker);

  std::vector<float> energies = simulator.run(frequency, numOfCollectors);
  positionTracker.saveAsJson();

  std::for_each(energies.begin(), energies.end(),
                [](float i) { std::cout << i << " "; });
  return 0;
}